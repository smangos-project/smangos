#include "MigrationPorter.h"

#include "MysqlDdlTranslator.h"
#include "MysqlLexer.h"
#include "SqlStatementSplitter.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string_view>
#include <unordered_map>

namespace
{
    std::string Lower(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    // One piece of a statement, at a granularity coarse enough to rewrite
    // CONCAT(...) calls and INSERT IGNORE without disturbing anything else:
    // every string/backtick-identifier/comment token from MysqlLexer stays
    // atomic (already re-escaped for StringLit, per world-sql.md's mapping),
    // and everything else is split into words, "(", ")" and ",".
    enum class AtomKind { Word, LParen, RParen, Comma, StringLit, Comment };
    struct Atom
    {
        AtomKind kind;
        std::string text;
    };

    // Decodes a MySQL string literal's quotes/escapes (DecodeMysqlStringLiteral
    // already handles both ' and " and every backslash escape) and re-emits it
    // as a SQLite literal: doubled '' for a literal quote, a raw newline for
    // \n, and `' || char(N) || '` pieces for \r, \0, \Z and \t so git's own
    // line-ending handling can never change the data (world-sql.md).
    std::string ReencodeStringLiteralForSqlite(std::string const& rawToken)
    {
        std::string decoded = DecodeMysqlStringLiteral(rawToken);
        std::string out = "'";
        for (unsigned char c : decoded)
        {
            switch (c)
            {
                case '\'': out += "''"; break;
                case '\n': out += '\n'; break;
                case '\r': out += "' || char(13) || '"; break;
                case '\0': out += "' || char(0) || '"; break;
                case '\x1a': out += "' || char(26) || '"; break; // \Z
                case '\t': out += "' || char(9) || '"; break;
                default: out += static_cast<char>(c); break;
            }
        }
        out += "'";
        return out;
    }

    void SplitOtherIntoAtoms(std::string const& text, std::vector<Atom>& atoms)
    {
        size_t i = 0;
        while (i < text.size())
        {
            char c = text[i];
            if (std::isspace(static_cast<unsigned char>(c)))
            {
                ++i;
                continue;
            }
            if (c == '(' || c == ')' || c == ',')
            {
                atoms.push_back({ c == '(' ? AtomKind::LParen : c == ')' ? AtomKind::RParen : AtomKind::Comma,
                    std::string(1, c) });
                ++i;
                continue;
            }
            size_t start = i;
            while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i])) &&
                   text[i] != '(' && text[i] != ')' && text[i] != ',')
                ++i;
            std::string word = text.substr(start, i - start);
            // MySQL's "&&" is always logical AND (never bitwise -- that's
            // "&"), unlike "||" (OR by default, but string concat under
            // PIPES_AS_CONCAT), so this direction is unambiguous. Found via
            // 166 real world migrations using it in a WHERE clause, which
            // SQLite has no "&&" operator to run at all.
            if (word == "&&")
                word = "AND";
            atoms.push_back({ AtomKind::Word, std::move(word) });
        }
    }

    std::vector<Atom> Atomize(std::string const& text)
    {
        std::vector<Atom> atoms;
        for (MysqlToken const& token : TokenizeMysql(text))
        {
            switch (token.type)
            {
                case MysqlTokenType::StringLiteral:
                    atoms.push_back({ AtomKind::StringLit, ReencodeStringLiteralForSqlite(token.text) });
                    break;
                case MysqlTokenType::QuotedIdentifier:
                    atoms.push_back({ AtomKind::Word, token.text });
                    break;
                case MysqlTokenType::Comment:
                case MysqlTokenType::VersionedComment:
                    atoms.push_back({ AtomKind::Comment, token.text });
                    break;
                case MysqlTokenType::Other:
                    SplitOtherIntoAtoms(token.text, atoms);
                    break;
                default:
                    break; // a DelimiterDirective doesn't belong inside one statement
            }
        }
        return atoms;
    }

    std::string AssembleAtoms(std::vector<Atom> const& atoms)
    {
        std::string out;
        bool first = true;
        bool afterLParen = false;
        for (Atom const& atom : atoms)
        {
            if (!first && !afterLParen && atom.kind != AtomKind::RParen && atom.kind != AtomKind::Comma)
                out += ' ';
            out += atom.text;
            if (atom.kind == AtomKind::Comment)
                out += '\n'; // a line comment must not swallow what follows
            first = false;
            afterLParen = atom.kind == AtomKind::LParen;
        }
        return out;
    }

    bool ParseInt64(std::string const& text, int64_t& value)
    {
        if (text.empty())
            return false;
        char* end = nullptr;
        long long v = std::strtoll(text.c_str(), &end, 10);
        if (end != text.c_str() + text.size())
            return false;
        value = v;
        return true;
    }

    // <int>, @name (a copy), or @name + <int> (an increment). Anything else
    // -- a subquery, a function call -- isn't evaluated: the caller flags it.
    bool EvaluateVariableRhs(std::string const& rhs, std::unordered_map<std::string, int64_t> const& variables, int64_t& value)
    {
        if (ParseInt64(rhs, value))
            return true;

        if (rhs.empty() || rhs.front() != '@')
            return false;

        size_t plusPos = rhs.find('+');
        std::string name = rhs.substr(1, plusPos == std::string::npos ? std::string::npos : plusPos - 1);
        while (!name.empty() && std::isspace(static_cast<unsigned char>(name.back())))
            name.pop_back();

        auto it = variables.find(name);
        if (it == variables.end())
            return false;

        if (plusPos == std::string::npos)
        {
            value = it->second;
            return true;
        }

        std::string delta = rhs.substr(plusPos + 1);
        while (!delta.empty() && std::isspace(static_cast<unsigned char>(delta.front())))
            delta.erase(delta.begin());
        int64_t deltaValue = 0;
        if (!ParseInt64(delta, deltaValue))
            return false;

        value = it->second + deltaValue;
        return true;
    }

    bool IsAtVariableAtom(Atom const& atom, std::string& name)
    {
        if (atom.kind != AtomKind::Word || atom.text.size() < 2 || atom.text[0] != '@')
            return false;
        for (size_t k = 1; k < atom.text.size(); ++k)
            if (!std::isalnum(static_cast<unsigned char>(atom.text[k])) && atom.text[k] != '_')
                return false;
        name = atom.text.substr(1);
        return true;
    }

    // "@name := <rhs>" used as a value, e.g. inside a VALUES tuple
    // ("(@POINT := @POINT + 1)"), which TryParseSetVariable never sees since
    // it isn't its own statement. Each occurrence is folded to a literal in
    // file order, updating `variables` exactly like a real per-row MySQL
    // evaluation would, so a counter that increments once per row comes out
    // right instead of every row getting the same value. An RHS
    // EvaluateVariableRhs can't evaluate is left as "name :=" for the caller
    // to flag, rather than emitted as broken SQL.
    bool RewriteInlineAssignmentAtoms(std::vector<Atom>& atoms, std::unordered_map<std::string, int64_t>& variables)
    {
        std::vector<Atom> out;
        size_t i = 0;
        while (i < atoms.size())
        {
            std::string name;
            if (IsAtVariableAtom(atoms[i], name) && i + 1 < atoms.size() &&
                atoms[i + 1].kind == AtomKind::Word && atoms[i + 1].text == ":=")
            {
                size_t j = i + 2;
                int depth = 0;
                std::vector<Atom> rhs;
                while (j < atoms.size())
                {
                    if (atoms[j].kind == AtomKind::LParen)
                        ++depth;
                    else if (atoms[j].kind == AtomKind::RParen)
                    {
                        if (depth == 0)
                            break;
                        --depth;
                    }
                    else if (atoms[j].kind == AtomKind::Comma && depth == 0)
                        break;
                    rhs.push_back(atoms[j]);
                    ++j;
                }

                int64_t value = 0;
                if (EvaluateVariableRhs(AssembleAtoms(rhs), variables, value))
                {
                    variables[name] = value;
                    out.push_back({ AtomKind::Word, std::to_string(value) });
                    i = j;
                    continue;
                }
                return false;
            }
            out.push_back(atoms[i]);
            ++i;
        }
        atoms = std::move(out);
        return true;
    }

    // @variables are plain characters to MysqlLexer, so "@X+1" is one Word
    // atom, not "@X" then "+1": substitution has to work inside a word, not
    // just replace whole-word matches.
    bool SubstituteVariablesInAtoms(std::vector<Atom>& atoms, std::unordered_map<std::string, int64_t> const& variables)
    {
        for (Atom& atom : atoms)
        {
            if (atom.kind != AtomKind::Word)
                continue;

            std::string const& word = atom.text;
            std::string rebuilt;
            size_t i = 0;
            while (i < word.size())
            {
                if (word[i] == '@' && i + 1 < word.size() &&
                    (std::isalpha(static_cast<unsigned char>(word[i + 1])) || word[i + 1] == '_'))
                {
                    size_t start = i + 1;
                    size_t j = start;
                    while (j < word.size() && (std::isalnum(static_cast<unsigned char>(word[j])) || word[j] == '_'))
                        ++j;
                    std::string name = word.substr(start, j - start);
                    auto it = variables.find(name);
                    if (it == variables.end())
                        return false;
                    rebuilt += std::to_string(it->second);
                    i = j;
                }
                else
                {
                    rebuilt += word[i];
                    ++i;
                }
            }
            atom.text = rebuilt;
        }
        return true;
    }

    // Replaces CONCAT(a, b, ...) with (a || b || ...), preserving everything
    // else -- comments, backtick identifiers, re-escaped strings -- exactly.
    // Only a flat (non-nested) argument list is handled, which covers every
    // real CONCAT call in the current migrations (world-sql.md: one file,
    // one call, two arguments); an unbalanced call is flagged rather than
    // silently mistranslated.
    bool RewriteConcatAtoms(std::vector<Atom>& atoms)
    {
        std::vector<Atom> out;
        size_t i = 0;
        while (i < atoms.size())
        {
            if (atoms[i].kind == AtomKind::Word && Lower(atoms[i].text) == "concat" &&
                i + 1 < atoms.size() && atoms[i + 1].kind == AtomKind::LParen)
            {
                size_t j = i + 2;
                int depth = 1;
                std::vector<std::vector<Atom>> args(1);
                while (j < atoms.size() && depth > 0)
                {
                    if (atoms[j].kind == AtomKind::LParen)
                    {
                        ++depth;
                        args.back().push_back(atoms[j]);
                    }
                    else if (atoms[j].kind == AtomKind::RParen)
                    {
                        --depth;
                        if (depth > 0)
                            args.back().push_back(atoms[j]);
                    }
                    else if (atoms[j].kind == AtomKind::Comma && depth == 1)
                    {
                        args.emplace_back();
                    }
                    else
                    {
                        args.back().push_back(atoms[j]);
                    }
                    ++j;
                }
                if (depth != 0)
                    return false;

                out.push_back({ AtomKind::LParen, "(" });
                for (size_t a = 0; a < args.size(); ++a)
                {
                    if (a)
                        out.push_back({ AtomKind::Word, "||" });
                    for (Atom const& sub : args[a])
                        out.push_back(sub);
                }
                out.push_back({ AtomKind::RParen, ")" });
                i = j;
                continue;
            }
            out.push_back(atoms[i]);
            ++i;
        }
        atoms = std::move(out);
        return true;
    }

    void RewriteInsertIgnoreAtoms(std::vector<Atom>& atoms)
    {
        // Skip a leading Comment atom: a statement is routinely preceded by
        // one glued onto its front (see SkipLeadingMysqlNoise), so "INSERT"
        // isn't reliably atoms[0].
        size_t i = 0;
        while (i < atoms.size() && atoms[i].kind == AtomKind::Comment)
            ++i;

        if (i + 1 < atoms.size() && atoms[i].kind == AtomKind::Word && Lower(atoms[i].text) == "insert" &&
            atoms[i + 1].kind == AtomKind::Word && Lower(atoms[i + 1].text) == "ignore")
        {
            atoms[i + 1].text = "OR IGNORE";
        }
    }

    bool IsMultiTableUpdate(std::string_view kind)
    {
        if (!StartsWithMysqlKeyword(kind, "UPDATE"))
            return false;

        std::vector<Atom> atoms = Atomize(std::string(kind));
        int depth = 0;
        for (size_t i = 1; i < atoms.size(); ++i)
        {
            if (atoms[i].kind == AtomKind::LParen) { ++depth; continue; }
            if (atoms[i].kind == AtomKind::RParen) { --depth; continue; }
            if (depth != 0)
                continue;
            if (atoms[i].kind == AtomKind::Comma)
                return true;
            if (atoms[i].kind == AtomKind::Word && Lower(atoms[i].text) == "set")
                return false;
        }
        return false;
    }

    // MySQL's "||" is logical OR under the default SQL_MODE but string
    // concatenation under PIPES_AS_CONCAT, while SQLite's is always
    // concatenation: unlike "&&" this direction is genuinely ambiguous (3
    // real files use it as OR), so it's flagged for a human to resolve
    // rather than guessed at. Checked before RewriteConcatAtoms introduces
    // its own (always-concatenation, so never ambiguous) "||"s, and only
    // against "Other" token text so a literal "||" inside a string or
    // comment is never mistaken for the operator.
    bool ContainsLogicalOrOperator(std::string const& text)
    {
        for (MysqlToken const& token : TokenizeMysql(text))
            if (token.type == MysqlTokenType::Other && token.text.find("||") != std::string::npos)
                return true;
        return false;
    }

    // Ports one already-split body statement (still MySQL dialect, still
    // possibly with a comment glued onto its front). Classification (which
    // kind of statement this is, and whether it needs a hand port) always
    // runs on the pristine original text: ContainsLogicalOrOperator in
    // particular would misfire on a "||" this same function introduces,
    // either from RewriteConcatAtoms or from re-escaping a string literal
    // that had a \r/\0/\Z/\t in it (world-sql.md's "' || char(N) || '"
    // pieces), if it ran on already-rewritten text instead.
    bool PortBodyStatement(std::string const& originalText, std::unordered_map<std::string, int64_t>& variables,
        std::string& rewrittenText, std::string& flagReason)
    {
        std::string_view kind = SkipLeadingMysqlNoise(originalText);

        if (StartsWithMysqlKeyword(kind, "ALTER TABLE"))
        {
            flagReason = "ALTER TABLE needs a hand port";
            return false;
        }

        if (ContainsLogicalOrOperator(originalText))
        {
            flagReason = "uses || (ambiguous MySQL OR vs SQLite concatenation), needs a hand port";
            return false;
        }

        if (StartsWithMysqlKeyword(kind, "CREATE TABLE"))
        {
            TranslatedTable translated = TranslateMysqlCreateTable(originalText);
            if (!translated.warnings.empty())
            {
                flagReason = translated.warnings.front();
                return false;
            }
            std::string joined;
            for (size_t i = 0; i < translated.statements.size(); ++i)
            {
                if (i)
                    joined += ";\n";
                joined += translated.statements[i];
            }
            rewrittenText = joined;
            return true;
        }

        if (StartsWithMysqlKeyword(kind, "TRUNCATE"))
        {
            for (MysqlToken const& token : TokenizeMysql(originalText))
            {
                if (token.type == MysqlTokenType::QuotedIdentifier)
                {
                    rewrittenText = "DELETE FROM " + token.text;
                    return true;
                }
            }
            flagReason = "TRUNCATE with no backtick-quoted table name";
            return false;
        }

        if (IsMultiTableUpdate(kind))
        {
            flagReason = "multi-table UPDATE needs a hand port";
            return false;
        }

        std::vector<Atom> atoms = Atomize(originalText);
        if (!RewriteInlineAssignmentAtoms(atoms, variables))
        {
            flagReason = "uses @variable := ... as a value expression, needs a hand port";
            return false;
        }
        if (!SubstituteVariablesInAtoms(atoms, variables))
        {
            flagReason = "references an unknown @variable";
            return false;
        }
        if (!RewriteConcatAtoms(atoms))
        {
            flagReason = "CONCAT(...) has an unbalanced argument list";
            return false;
        }
        RewriteInsertIgnoreAtoms(atoms);
        rewrittenText = AssembleAtoms(atoms);
        return true;
    }

    // "SET @X := 5", "SET @X = @Y" or "SET @X = @X + 5". Returns false (not
    // an assignment at all) for anything else, including the envelope's own
    // "SET v = (SELECT ...)" (no leading '@') and "SET NAMES ...".
    bool TryParseSetVariable(std::string_view kind, std::string& varName, std::string& rhs)
    {
        if (!StartsWithMysqlKeyword(kind, "SET"))
            return false;

        size_t i = 3;
        while (i < kind.size() && std::isspace(static_cast<unsigned char>(kind[i])))
            ++i;
        if (i >= kind.size() || kind[i] != '@')
            return false;

        size_t nameStart = ++i;
        while (i < kind.size() && (std::isalnum(static_cast<unsigned char>(kind[i])) || kind[i] == '_'))
            ++i;
        varName = std::string(kind.substr(nameStart, i - nameStart));

        while (i < kind.size() && std::isspace(static_cast<unsigned char>(kind[i])))
            ++i;
        if (i < kind.size() && kind[i] == ':')
            ++i;
        if (i >= kind.size() || kind[i] != '=')
            return false;
        ++i;

        while (i < kind.size() && std::isspace(static_cast<unsigned char>(kind[i])))
            ++i;
        size_t rhsEnd = kind.size();
        while (rhsEnd > i && std::isspace(static_cast<unsigned char>(kind[rhsEnd - 1])))
            --rhsEnd;
        rhs = std::string(kind.substr(i, rhsEnd - i));
        return true;
    }

}

PortedMigration PortMysqlMigration(std::string const& id, std::string const& fileContent)
{
    PortedMigration result;
    auto flag = [&](std::string reason)
    {
        result.flagged = true;
        result.flagReason = std::move(reason);
        return result;
    };

    std::vector<SqlScriptStatement> statements = SplitMysqlStatements(fileContent);
    auto procedureIt = std::find_if(statements.begin(), statements.end(), [](SqlScriptStatement const& s)
    {
        return StartsWithMysqlKeyword(SkipLeadingMysqlNoise(s.text), "CREATE PROCEDURE");
    });
    if (procedureIt == statements.end())
        return flag("no CREATE PROCEDURE envelope found");

    std::string const& procedureText = procedureIt->text;
    std::string procedureLower = Lower(procedureText);

    // Almost every file quotes its own id ('20210515110157'); at least one
    // real file (20260426065508) uses a bare numeric literal instead.
    std::string quotedMarker = Lower("migrations` VALUES ('" + id + "')");
    std::string bareMarker = Lower("migrations` VALUES (" + id + ")");
    size_t markerPos = procedureLower.find(quotedMarker);
    if (markerPos == std::string::npos)
        markerPos = procedureLower.find(bareMarker);
    if (markerPos == std::string::npos)
        return flag("could not find the envelope's INSERT INTO migrations marker for id " + id);

    size_t bodyStart = procedureLower.rfind("insert", markerPos);
    if (bodyStart == std::string::npos)
        return flag("could not find INSERT before the migrations marker");

    size_t endIfPos = procedureLower.rfind("end if");
    if (endIfPos == std::string::npos || endIfPos < bodyStart)
        return flag("could not find the envelope's closing END IF");

    std::string body = procedureText.substr(bodyStart, endIfPos - bodyStart);
    size_t lastNonSpace = body.find_last_not_of(" \t\r\n");
    body = lastNonSpace == std::string::npos ? std::string() : body.substr(0, lastNonSpace + 1);

    std::vector<SqlScriptStatement> bodyStatements = SplitMysqlStatements(body);
    if (bodyStatements.empty())
        return flag("the migration body is empty");

    std::unordered_map<std::string, int64_t> variables;
    std::string output = "INSERT INTO `migrations` VALUES ('" + id + "');\n\n";

    for (size_t i = 1; i < bodyStatements.size(); ++i)
    {
        SqlScriptStatement const& statement = bodyStatements[i];
        std::string_view kind = SkipLeadingMysqlNoise(statement.text);

        std::string varName, rhs;
        if (TryParseSetVariable(kind, varName, rhs))
        {
            int64_t value = 0;
            if (!EvaluateVariableRhs(rhs, variables, value))
                return flag("line " + std::to_string(statement.line) + ": unsupported @" + varName + " assignment");
            variables[varName] = value;
            continue;
        }

        // SQLite has no such statement; world-sql.md removes it, same as the
        // base dump importer already does for the dump's own SET NAMES lines.
        if (StartsWithMysqlKeyword(kind, "SET NAMES"))
            continue;

        std::string rewritten, flagReason;
        if (!PortBodyStatement(statement.text, variables, rewritten, flagReason))
            return flag("line " + std::to_string(statement.line) + ": " + flagReason);

        output += rewritten;
        output += ";\n\n";
    }

    while (!output.empty() && output.back() == '\n')
        output.pop_back();
    output += "\n";

    result.content = output;
    return result;
}

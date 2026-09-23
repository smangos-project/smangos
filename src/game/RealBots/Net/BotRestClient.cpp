#include "BotRestClient.h"
#include "BotConnector.h"
#include "Errors.h"
#include "Log.h"
#include "Util.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>

using json = nlohmann::json;

namespace
{
std::vector<uint8> HexDecode(std::string const& hex)
{
    if (hex.empty() || (hex.size() % 2) != 0)
        return {};
    std::vector<uint8> out(hex.size() / 2);
    HexStrToByteArray(hex, out.data());
    return out;
}

std::string HexEncode(std::vector<uint8> const& bytes)
{
    std::string hex;
    hexEncodeByteArray(const_cast<uint8*>(bytes.data()), uint32(bytes.size()), hex);
    return hex;
}

// Matches LoginRESTService's ParseFormInputs: {"inputs":[{"input_id":"...","value":"..."}]}.
std::string BuildFormInputsJson(std::vector<std::pair<std::string, std::string>> const& inputs)
{
    json body;
    body["inputs"] = json::array();
    for (auto const& [id, value] : inputs)
        body["inputs"].push_back({ { "input_id", id }, { "value", value } });
    return body.dump();
}

// Real Battle.net clients uppercase (Latin-only) both the account name and password before any
// SRP math -- vibemangos's own account provisioning does the same
// (AccountMgr::CreateAccount's normalizeString calls; LoginRESTService::NormalizeAccountString
// mirrors it for the account name at login time). The account name doesn't strictly need this
// here since the server independently normalizes whatever it receives and echoes back the
// resulting srpUsername either way (see OnSrpChallengeResponse), but the password never crosses
// the wire in plaintext during login: there's no server-side chance to fix a case mismatch, so a
// password normalized to uppercase at account-creation time will never produce a matching M1
// unless this side uppercases it too first. Confirmed against a live vibemangos instance this
// session -- a lowercase-typed password that logs into the real game client (which does this
// same normalization) was otherwise rejected here.
std::string NormalizeAccountString(std::string const& utf8str)
{
    std::wstring wstr;
    if (!Utf8toWStr(utf8str, wstr))
        return utf8str;
    std::transform(wstr.begin(), wstr.end(), wstr.begin(), wcharToUpperOnlyLatin);
    std::string out;
    if (!WStrToUtf8(wstr, out))
        return utf8str;
    return out;
}
}

std::shared_ptr<BotRestClient> BotRestClient::Create(IO::Networking::AsyncSocket socket)
{
    return std::shared_ptr<BotRestClient>(new BotRestClient(std::move(socket)));
}

BotRestClient::BotRestClient(IO::Networking::AsyncSocket socket)
{
    m_stream = BotTlsStream::Create(std::move(socket));
}

void BotRestClient::Login(std::string const& accountName, std::string const& password, LoginCallback callback)
{
    m_accountName = NormalizeAccountString(accountName);
    m_password = NormalizeAccountString(password);
    m_callback = std::move(callback);

    std::shared_ptr<BotRestClient> self = shared_from_this();
    m_stream->Connect([self](bool success) { self->OnConnected(success); });
}

void BotRestClient::OnConnected(bool success)
{
    if (!success)
    {
        Fail("TLS handshake with the login portal failed");
        return;
    }

    std::string body = BuildFormInputsJson({ { "account_name", m_accountName } });
    std::shared_ptr<BotRestClient> self = shared_from_this();
    PostFormInputs("/bnetserver/login/srp/", body, [self](bool ok, HttpResponse response)
    {
        self->OnSrpChallengeResponse(ok, std::move(response));
    });
}

void BotRestClient::OnSrpChallengeResponse(bool ok, HttpResponse response)
{
    if (!ok)
    {
        Fail("no response to the SRP challenge request");
        return;
    }

    json challenge;
    try
    {
        challenge = json::parse(response.Body);
    }
    catch (json::exception const&)
    {
        Fail("malformed SRP challenge response: " + response.Body);
        return;
    }

    if (response.Status != 200)
    {
        Fail("SRP challenge request failed (" + std::to_string(response.Status) + "): " +
            challenge.value("error_message", response.Body));
        return;
    }

    std::vector<uint8> salt = HexDecode(challenge.value("salt", std::string()));
    std::vector<uint8> hostPublicEphemeralB = HexDecode(challenge.value("public_B", std::string()));
    std::string srpUsername = challenge.value("username", std::string());
    if (salt.empty() || hostPublicEphemeralB.empty() || srpUsername.empty())
    {
        Fail("SRP challenge response missing salt/public_B/username");
        return;
    }

    m_srp.CalculateClientPublicEphemeral();
    std::vector<uint8> clientEvidenceM1 = m_srp.CalculateClientEvidence(srpUsername, m_password, salt, hostPublicEphemeralB);
    if (clientEvidenceM1.empty())
    {
        Fail("SRP challenge response had a degenerate public_B");
        return;
    }

    std::string body = BuildFormInputsJson({
        { "public_A", HexEncode(m_srp.GetClientPublicEphemeral()) },
        { "client_evidence_M1", HexEncode(clientEvidenceM1) },
    });

    std::shared_ptr<BotRestClient> self = shared_from_this();
    PostFormInputs("/bnetserver/login/", body, [self](bool ok, HttpResponse response)
    {
        self->OnSrpProofResponse(ok, std::move(response));
    });
}

void BotRestClient::OnSrpProofResponse(bool ok, HttpResponse response)
{
    if (!ok)
    {
        Fail("no response to the SRP proof request");
        return;
    }

    json result;
    try
    {
        result = json::parse(response.Body);
    }
    catch (json::exception const&)
    {
        Fail("malformed SRP proof response: " + response.Body);
        return;
    }

    if (response.Status != 200)
    {
        Fail("SRP proof request failed (" + std::to_string(response.Status) + "): " +
            result.value("error_message", response.Body));
        return;
    }

    std::vector<uint8> serverEvidenceM2 = HexDecode(result.value("server_evidence_M2", std::string()));
    if (serverEvidenceM2.empty() || !m_srp.VerifyServerEvidence(serverEvidenceM2))
    {
        Fail("server evidence M2 didn't match -- possible impostor server");
        return;
    }

    std::string ticket = result.value("login_ticket", std::string());
    if (ticket.empty())
    {
        Fail("SRP proof response had no login_ticket");
        return;
    }

    Succeed(ticket);
}

void BotRestClient::PostFormInputs(std::string const& path, std::string const& jsonBody, HttpCallback callback)
{
    std::ostringstream request;
    request << "POST " << path << " HTTP/1.1\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << jsonBody.size() << "\r\n"
        << "Connection: keep-alive\r\n"
        << "\r\n"
        << jsonBody;
    std::string requestString = request.str();

    std::shared_ptr<BotRestClient> self = shared_from_this();
    m_stream->Write(std::vector<uint8>(requestString.begin(), requestString.end()), [self, callback](IO::NetworkError const& error)
    {
        if (error)
        {
            callback(false, {});
            return;
        }

        self->ReadStatusAndHeaders([self, callback](bool ok, int status, size_t contentLength)
        {
            if (!ok)
            {
                callback(false, {});
                return;
            }

            self->ReadBody(contentLength, [callback, status](bool ok, std::string body)
            {
                if (!ok)
                {
                    callback(false, {});
                    return;
                }
                callback(true, HttpResponse{ status, std::move(body) });
            });
        });
    });
}

void BotRestClient::ReadLine(std::shared_ptr<std::string> line, std::function<void(bool, std::string)> callback)
{
    auto byte = std::make_shared<char>('\0');
    std::shared_ptr<BotRestClient> self = shared_from_this();
    m_stream->Read(byte.get(), 1, [self, byte, line, callback](IO::NetworkError const& error, size_t)
    {
        if (error)
        {
            callback(false, std::string());
            return;
        }

        if (*byte == '\n')
        {
            if (!line->empty() && line->back() == '\r')
                line->pop_back();
            callback(true, *line);
            return;
        }

        line->push_back(*byte);
        self->ReadLine(line, callback);
    });
}

void BotRestClient::ReadStatusAndHeaders(std::function<void(bool, int, size_t)> callback)
{
    std::shared_ptr<BotRestClient> self = shared_from_this();
    ReadLine(std::make_shared<std::string>(), [self, callback](bool ok, std::string statusLine)
    {
        if (!ok)
        {
            callback(false, 0, 0);
            return;
        }

        std::istringstream iss(statusLine);
        std::string version;
        int status = 0;
        if (!(iss >> version >> status))
        {
            sLog.Out(LOG_NETWORK, LOG_LVL_ERROR, "BotRestClient: malformed status line '%s'", statusLine.c_str());
            callback(false, 0, 0);
            return;
        }

        auto contentLength = std::make_shared<size_t>(0);

        // Reads header lines until the blank line separator, tracking Content-Length. Each
        // readHeader call reads one line and, unless it was the blank separator, schedules
        // itself again -- kept alive across calls via its own shared_ptr since it captures
        // itself by value.
        std::shared_ptr<std::function<void()>> readHeader = std::make_shared<std::function<void()>>();
        *readHeader = [self, callback, status, contentLength, readHeader]()
        {
            self->ReadLine(std::make_shared<std::string>(), [self, callback, status, contentLength, readHeader](bool ok, std::string line)
            {
                if (!ok)
                {
                    callback(false, 0, 0);
                    return;
                }

                if (line.empty())
                {
                    callback(true, status, *contentLength);
                    return;
                }

                constexpr char const* prefix = "content-length:";
                if (line.size() > std::strlen(prefix))
                {
                    std::string lowered = line.substr(0, std::strlen(prefix));
                    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return std::tolower(c); });
                    if (lowered == prefix)
                    {
                        size_t valueStart = line.find_first_not_of(" \t", std::strlen(prefix));
                        if (valueStart != std::string::npos)
                            *contentLength = size_t(strtoull(line.c_str() + valueStart, nullptr, 10));
                    }
                }

                (*readHeader)();
            });
        };
        (*readHeader)();
    });
}

void BotRestClient::ReadBody(size_t contentLength, std::function<void(bool, std::string)> callback)
{
    if (contentLength == 0)
    {
        callback(true, std::string());
        return;
    }

    auto body = std::make_shared<std::string>(contentLength, '\0');
    std::shared_ptr<BotRestClient> self = shared_from_this();
    m_stream->Read(body->data(), body->size(), [callback, body](IO::NetworkError const& error, size_t)
    {
        callback(!error, error ? std::string() : *body);
    });
}

void BotRestClient::Fail(std::string const& error)
{
    sLog.Out(LOG_NETWORK, LOG_LVL_BASIC, "BotRestClient: %s", error.c_str());
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        LoginResult result;
        result.Success = false;
        result.Error = error;
        callback(std::move(result));
    }
}

void BotRestClient::Succeed(std::string const& ticket)
{
    if (m_callback)
    {
        LoginCallback callback = std::move(m_callback);
        m_callback = nullptr;
        LoginResult result;
        result.Success = true;
        result.LoginTicket = ticket;
        callback(std::move(result));
    }
}

void BotRestClient::Close()
{
    m_stream->Close();
}

#include "HttpRequestParser.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace
{
std::string ToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return s;
}
}

HttpRequestParser::HttpRequestParser(size_t maxHeaderBytes, size_t maxBodyBytes)
    : m_maxHeaderBytes(maxHeaderBytes), m_maxBodyBytes(maxBodyBytes)
{
}

HttpRequestParser::Result HttpRequestParser::Feed(char const* data, size_t size)
{
    if (m_errorStatus)
        return Result::Malformed;

    m_buffer.append(data, size);
    return Parse();
}

HttpRequestParser::Result HttpRequestParser::Next()
{
    if (m_errorStatus)
        return Result::Malformed;

    if (m_complete)
        m_buffer.erase(0, m_headerEnd + m_bodyLength);

    m_request = HttpRequest{};
    m_headerEnd = 0;
    m_bodyLength = 0;
    m_headersParsed = false;
    m_complete = false;
    return Parse();
}

bool HttpRequestParser::KeepAlive() const
{
    if (m_request.Line.Version == "HTTP/1.0")
        return false;

    nonstd::optional<std::string> connection = HttpMessage::FindHeader(m_request.Headers, "connection");
    return !connection || ToLower(*connection) != "close";
}

HttpRequestParser::Result HttpRequestParser::Parse()
{
    if (m_complete)
        return Result::Complete;

    if (!m_headersParsed)
    {
        // Tolerate blank lines between pipelined requests.
        size_t const first = m_buffer.find_first_not_of("\r\n");
        if (first == std::string::npos)
        {
            m_buffer.clear();
            return Result::NeedMore;
        }
        if (first > 0)
            m_buffer.erase(0, first);

        size_t const blank = m_buffer.find("\r\n\r\n");
        if (blank == std::string::npos)
        {
            if (m_buffer.size() > m_maxHeaderBytes)
            {
                m_errorStatus = 431;
                return Result::Malformed;
            }
            return Result::NeedMore;
        }
        if (blank + 4 > m_maxHeaderBytes)
        {
            m_errorStatus = 431;
            return Result::Malformed;
        }

        size_t lineStart = 0;
        size_t lineEnd = m_buffer.find("\r\n");
        nonstd::optional<HttpMessage::RequestLine> requestLine = HttpMessage::ParseRequestLine(m_buffer.substr(0, lineEnd));
        if (!requestLine || requestLine->Version.compare(0, 5, "HTTP/") != 0)
        {
            m_errorStatus = 400;
            return Result::Malformed;
        }
        m_request.Line = *requestLine;

        lineStart = lineEnd + 2;
        while (lineStart < blank)
        {
            lineEnd = m_buffer.find("\r\n", lineStart);
            nonstd::optional<HttpMessage::Header> header = HttpMessage::ParseHeaderLine(m_buffer.substr(lineStart, lineEnd - lineStart));
            if (!header)
            {
                m_errorStatus = 400;
                return Result::Malformed;
            }
            m_request.Headers.push_back(std::move(*header));
            lineStart = lineEnd + 2;
        }

        if (HttpMessage::FindHeader(m_request.Headers, "transfer-encoding"))
        {
            m_errorStatus = 501;
            return Result::Malformed;
        }

        m_bodyLength = 0;
        if (nonstd::optional<std::string> length = HttpMessage::FindHeader(m_request.Headers, "content-length"))
        {
            if (length->empty() || length->find_first_not_of("0123456789") != std::string::npos || length->size() > 9)
            {
                m_errorStatus = 400;
                return Result::Malformed;
            }
            m_bodyLength = size_t(std::strtoul(length->c_str(), nullptr, 10));
            if (m_bodyLength > m_maxBodyBytes)
            {
                m_errorStatus = 413;
                return Result::Malformed;
            }
        }

        m_headerEnd = blank + 4;
        m_headersParsed = true;
    }

    if (m_buffer.size() < m_headerEnd + m_bodyLength)
        return Result::NeedMore;

    m_request.Body = m_buffer.substr(m_headerEnd, m_bodyLength);
    m_complete = true;
    return Result::Complete;
}

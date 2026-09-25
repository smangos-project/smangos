#include "HttpMessage.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
std::string ToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return s;
}
}

std::string HttpMessage::BuildRequest(std::string const& method, std::string const& path,
    std::vector<Header> const& headers, std::string const& body)
{
    std::ostringstream out;
    out << method << " " << path << " HTTP/1.1\r\n";
    for (Header const& header : headers)
        out << header.Name << ": " << header.Value << "\r\n";
    out << "\r\n" << body;
    return out.str();
}

std::string HttpMessage::BuildResponse(int status, std::string const& reason,
    std::vector<Header> const& headers, std::string const& body)
{
    std::ostringstream out;
    out << "HTTP/1.1 " << status << " " << reason << "\r\n";
    for (Header const& header : headers)
        out << header.Name << ": " << header.Value << "\r\n";
    out << "\r\n" << body;
    return out.str();
}

nonstd::optional<HttpMessage::RequestLine> HttpMessage::ParseRequestLine(std::string const& line)
{
    std::istringstream iss(line);
    RequestLine out;
    if (!(iss >> out.Method >> out.Path >> out.Version))
        return nonstd::nullopt;
    return out;
}

nonstd::optional<HttpMessage::StatusLine> HttpMessage::ParseStatusLine(std::string const& line)
{
    std::istringstream iss(line);
    StatusLine out;
    if (!(iss >> out.Version >> out.Status))
        return nonstd::nullopt;
    std::getline(iss, out.Reason);
    size_t const start = out.Reason.find_first_not_of(" \t");
    out.Reason = (start == std::string::npos) ? std::string() : out.Reason.substr(start);
    return out;
}

nonstd::optional<HttpMessage::Header> HttpMessage::ParseHeaderLine(std::string const& line)
{
    size_t const colon = line.find(':');
    if (colon == std::string::npos)
        return nonstd::nullopt;

    Header out;
    out.Name = line.substr(0, colon);
    size_t const valueStart = line.find_first_not_of(" \t", colon + 1);
    out.Value = (valueStart == std::string::npos) ? std::string() : line.substr(valueStart);
    return out;
}

nonstd::optional<std::string> HttpMessage::FindHeader(std::vector<Header> const& headers, std::string const& name)
{
    std::string const lowered = ToLower(name);
    for (Header const& header : headers)
        if (ToLower(header.Name) == lowered)
            return header.Value;
    return nonstd::nullopt;
}

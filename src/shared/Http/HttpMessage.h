#ifndef SHARED_HTTPMESSAGE_H
#define SHARED_HTTPMESSAGE_H

#include "nonstd/optional.hpp"
#include <string>
#include <vector>

// A minimal HTTP/1.1 message codec, no socket I/O: build a whole request or response as one
// string, and parse a request/status line or a header line handed to it one at a time (a
// caller reads lines off its own transport since a body's length isn't known until
// Content-Length has been parsed out of the headers).
namespace HttpMessage
{
struct Header
{
    std::string Name;
    std::string Value;
};

struct RequestLine
{
    std::string Method;
    std::string Path;
    std::string Version;
};

struct StatusLine
{
    std::string Version;
    int Status = 0;
    std::string Reason;
};

std::string BuildRequest(std::string const& method, std::string const& path,
    std::vector<Header> const& headers, std::string const& body);

std::string BuildResponse(int status, std::string const& reason,
    std::vector<Header> const& headers, std::string const& body);

nonstd::optional<RequestLine> ParseRequestLine(std::string const& line);
nonstd::optional<StatusLine> ParseStatusLine(std::string const& line);

// A blank line (the header/body separator) isn't a header and returns nullopt; callers stop
// reading headers on a blank line themselves rather than relying on this to signal it.
nonstd::optional<Header> ParseHeaderLine(std::string const& line);

// Case-insensitive name match, first hit wins.
nonstd::optional<std::string> FindHeader(std::vector<Header> const& headers, std::string const& name);
}

#endif

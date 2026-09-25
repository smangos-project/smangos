#include "Http/HttpMessage.h"

#include <catch_amalgamated.hpp>

namespace
{
// Splits a built message into its lines (up to the blank separator) and the body that follows,
// the way a line-at-a-time socket reader would see it.
struct SplitMessage
{
    std::vector<std::string> Lines;
    std::string Body;
};

SplitMessage SplitOnCrlf(std::string const& message)
{
    SplitMessage out;
    size_t pos = 0;
    for (;;)
    {
        size_t const end = message.find("\r\n", pos);
        REQUIRE(end != std::string::npos);
        std::string line = message.substr(pos, end - pos);
        pos = end + 2;
        if (line.empty())
            break;
        out.Lines.push_back(std::move(line));
    }
    out.Body = message.substr(pos);
    return out;
}
}

TEST_CASE("HttpMessage round-trips a built request", "[http]")
{
    std::string const message = HttpMessage::BuildRequest("POST", "/bnetserver/login/",
        { { "Content-Type", "application/json" }, { "Content-Length", "5" } }, "hello");

    SplitMessage const split = SplitOnCrlf(message);
    REQUIRE(split.Lines.size() == 3);

    nonstd::optional<HttpMessage::RequestLine> requestLine = HttpMessage::ParseRequestLine(split.Lines[0]);
    REQUIRE(requestLine.has_value());
    CHECK(requestLine->Method == "POST");
    CHECK(requestLine->Path == "/bnetserver/login/");
    CHECK(requestLine->Version == "HTTP/1.1");

    std::vector<HttpMessage::Header> headers;
    for (size_t i = 1; i < split.Lines.size(); ++i)
    {
        nonstd::optional<HttpMessage::Header> header = HttpMessage::ParseHeaderLine(split.Lines[i]);
        REQUIRE(header.has_value());
        headers.push_back(*header);
    }

    CHECK(HttpMessage::FindHeader(headers, "content-type") == "application/json");
    CHECK(HttpMessage::FindHeader(headers, "Content-Length") == "5");
    CHECK(split.Body == "hello");
}

TEST_CASE("HttpMessage round-trips a built response", "[http]")
{
    std::string const message = HttpMessage::BuildResponse(200, "OK",
        { { "Content-Length", "13" }, { "Set-Cookie", "session=abc; Path=/" } }, "{\"a\":\"json\"}");

    SplitMessage const split = SplitOnCrlf(message);
    REQUIRE(split.Lines.size() == 3);

    nonstd::optional<HttpMessage::StatusLine> statusLine = HttpMessage::ParseStatusLine(split.Lines[0]);
    REQUIRE(statusLine.has_value());
    CHECK(statusLine->Version == "HTTP/1.1");
    CHECK(statusLine->Status == 200);
    CHECK(statusLine->Reason == "OK");

    std::vector<HttpMessage::Header> headers;
    for (size_t i = 1; i < split.Lines.size(); ++i)
    {
        nonstd::optional<HttpMessage::Header> header = HttpMessage::ParseHeaderLine(split.Lines[i]);
        REQUIRE(header.has_value());
        headers.push_back(*header);
    }

    CHECK(HttpMessage::FindHeader(headers, "content-length") == "13");
    // Case-insensitive on the name; the cookie's own value (with its ';' attributes) is opaque.
    CHECK(HttpMessage::FindHeader(headers, "set-cookie") == "session=abc; Path=/");
    CHECK(split.Body == "{\"a\":\"json\"}");
}

TEST_CASE("HttpMessage::ParseHeaderLine rejects a line with no colon", "[http]")
{
    CHECK_FALSE(HttpMessage::ParseHeaderLine("not-a-header").has_value());
}

TEST_CASE("HttpMessage::ParseRequestLine and ParseStatusLine reject a malformed line", "[http]")
{
    CHECK_FALSE(HttpMessage::ParseRequestLine("").has_value());
    CHECK_FALSE(HttpMessage::ParseStatusLine("garbage").has_value());
}

TEST_CASE("HttpMessage::FindHeader returns nullopt when the header is absent", "[http]")
{
    std::vector<HttpMessage::Header> const headers = { { "Content-Type", "text/plain" } };
    CHECK_FALSE(HttpMessage::FindHeader(headers, "content-length").has_value());
}

#include "Http/HttpRequestParser.h"

#include <catch_amalgamated.hpp>

namespace
{
HttpRequestParser::Result FeedString(HttpRequestParser& parser, std::string const& text)
{
    return parser.Feed(text.data(), text.size());
}
}

TEST_CASE("HttpRequestParser reads a request without a body", "[http]")
{
    HttpRequestParser parser;
    REQUIRE(FeedString(parser, "GET /bnetserver/login/ HTTP/1.1\r\nHost: example\r\n\r\n") == HttpRequestParser::Result::Complete);

    CHECK(parser.Request().Line.Method == "GET");
    CHECK(parser.Request().Line.Path == "/bnetserver/login/");
    CHECK(HttpMessage::FindHeader(parser.Request().Headers, "host") == "example");
    CHECK(parser.Request().Body.empty());
    CHECK(parser.KeepAlive());
}

TEST_CASE("HttpRequestParser reads a request built by HttpMessage, fed one byte at a time", "[http]")
{
    std::string const body = "{\"inputs\":[]}";
    std::string const message = HttpMessage::BuildRequest("POST", "/bnetserver/login/srp/",
        { { "Content-Type", "application/json" }, { "Content-Length", std::to_string(body.size()) } }, body);

    HttpRequestParser parser;
    HttpRequestParser::Result result = HttpRequestParser::Result::NeedMore;
    for (size_t i = 0; i < message.size(); ++i)
    {
        CHECK(result == HttpRequestParser::Result::NeedMore);
        result = parser.Feed(&message[i], 1);
    }

    REQUIRE(result == HttpRequestParser::Result::Complete);
    CHECK(parser.Request().Line.Method == "POST");
    CHECK(parser.Request().Line.Path == "/bnetserver/login/srp/");
    CHECK(parser.Request().Body == body);
}

TEST_CASE("HttpRequestParser hands back pipelined requests one at a time", "[http]")
{
    std::string const first = HttpMessage::BuildRequest("POST", "/a", { { "Content-Length", "3" } }, "abc");
    std::string const second = HttpMessage::BuildRequest("GET", "/b", {}, "");

    HttpRequestParser parser;
    REQUIRE(FeedString(parser, first + second) == HttpRequestParser::Result::Complete);
    CHECK(parser.Request().Line.Path == "/a");
    CHECK(parser.Request().Body == "abc");

    REQUIRE(parser.Next() == HttpRequestParser::Result::Complete);
    CHECK(parser.Request().Line.Path == "/b");
    CHECK(parser.Request().Body.empty());

    CHECK(parser.Next() == HttpRequestParser::Result::NeedMore);
}

TEST_CASE("HttpRequestParser waits for the rest of a body", "[http]")
{
    HttpRequestParser parser;
    CHECK(FeedString(parser, "POST /a HTTP/1.1\r\nContent-Length: 10\r\n\r\n12345") == HttpRequestParser::Result::NeedMore);
    REQUIRE(FeedString(parser, "67890") == HttpRequestParser::Result::Complete);
    CHECK(parser.Request().Body == "1234567890");
}

TEST_CASE("HttpRequestParser rejects malformed requests with a status", "[http]")
{
    SECTION("a request line that is not HTTP")
    {
        HttpRequestParser parser;
        CHECK(FeedString(parser, "hello world\r\n\r\n") == HttpRequestParser::Result::Malformed);
        CHECK(parser.ErrorStatus() == 400);
    }

    SECTION("a header without a colon")
    {
        HttpRequestParser parser;
        CHECK(FeedString(parser, "GET / HTTP/1.1\r\nnot a header\r\n\r\n") == HttpRequestParser::Result::Malformed);
        CHECK(parser.ErrorStatus() == 400);
    }

    SECTION("a Content-Length that is not a number")
    {
        HttpRequestParser parser;
        CHECK(FeedString(parser, "POST / HTTP/1.1\r\nContent-Length: abc\r\n\r\n") == HttpRequestParser::Result::Malformed);
        CHECK(parser.ErrorStatus() == 400);
    }

    SECTION("a body over the limit")
    {
        HttpRequestParser parser(1024, 16);
        CHECK(FeedString(parser, "POST / HTTP/1.1\r\nContent-Length: 17\r\n\r\n") == HttpRequestParser::Result::Malformed);
        CHECK(parser.ErrorStatus() == 413);
    }

    SECTION("a header block over the limit, before it ends")
    {
        HttpRequestParser parser(64, 16);
        CHECK(FeedString(parser, std::string(200, 'a')) == HttpRequestParser::Result::Malformed);
        CHECK(parser.ErrorStatus() == 431);
    }

    SECTION("a chunked body")
    {
        HttpRequestParser parser;
        CHECK(FeedString(parser, "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n") == HttpRequestParser::Result::Malformed);
        CHECK(parser.ErrorStatus() == 501);
    }
}

TEST_CASE("HttpRequestParser reports whether the connection stays open", "[http]")
{
    {
        HttpRequestParser parser;
        REQUIRE(FeedString(parser, "GET / HTTP/1.1\r\nConnection: close\r\n\r\n") == HttpRequestParser::Result::Complete);
        CHECK_FALSE(parser.KeepAlive());
    }
    {
        HttpRequestParser parser;
        REQUIRE(FeedString(parser, "GET / HTTP/1.0\r\n\r\n") == HttpRequestParser::Result::Complete);
        CHECK_FALSE(parser.KeepAlive());
    }
}

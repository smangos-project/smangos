#ifndef SHARED_HTTPREQUESTPARSER_H
#define SHARED_HTTPREQUESTPARSER_H

#include "Http/HttpMessage.h"

#include <cstddef>
#include <string>
#include <vector>

struct HttpRequest
{
    HttpMessage::RequestLine Line;
    std::vector<HttpMessage::Header> Headers;
    std::string Body;
};

// Incremental HTTP/1.1 request reader for a server: feed it whatever bytes arrived, and it says
// when a whole request is there. A request is a header block plus a Content-Length body; chunked
// bodies are not supported. Bytes past the end of a request stay buffered for the next one.
class HttpRequestParser
{
    public:
        enum class Result
        {
            NeedMore,
            Complete,   // Request() is valid until Next()
            Malformed,  // ErrorStatus() is the status to answer with
        };

        HttpRequestParser(size_t maxHeaderBytes = 16 * 1024, size_t maxBodyBytes = 64 * 1024);

        Result Feed(char const* data, size_t size);

        HttpRequest const& Request() const { return m_request; }
        int ErrorStatus() const { return m_errorStatus; }

        // Drops the finished request and parses again from any bytes already buffered behind it.
        Result Next();

        // False when the request asked to close the connection after its response, or is HTTP/1.0.
        bool KeepAlive() const;

    private:
        Result Parse();

        std::string m_buffer;
        HttpRequest m_request;
        size_t m_headerEnd = 0;   // offset just past the blank line, once the header block is parsed
        size_t m_bodyLength = 0;
        bool m_headersParsed = false;
        bool m_complete = false;
        int m_errorStatus = 0;
        size_t m_maxHeaderBytes;
        size_t m_maxBodyBytes;
};

#endif

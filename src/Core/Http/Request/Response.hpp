#ifndef CORE_HTTP_REQUEST_RESPONSE_HPP
#define CORE_HTTP_REQUEST_RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>

namespace Http
{
    class Response
    {
    public:
        int statusCode = 200;
        std::string statusMessage = "OK";
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        void setStatus(int code, const std::string &message)
        {
            statusCode = code;
            statusMessage = message;
        }

        void setHeader(const std::string &key, const std::string &value)
        {
            headers[key] = value;
        }

        std::string toString() const
        {
            std::ostringstream responseStream;
            responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
            for (const auto &[key, val] : headers)
            {
                responseStream << key << ": " << val << "\r\n";
            }
            responseStream << "Content-Length: " << body.size() << "\r\n";
            responseStream << "Connection: keep-alive" << "\r\n\r\n";
            responseStream << body;
            return responseStream.str();
        }
    };
}

#endif
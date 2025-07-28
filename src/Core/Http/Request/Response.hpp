#ifndef CORE_HTTP_REQUEST_RESPONSE_HPP
#define CORE_HTTP_REQUEST_RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <regex>

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

        Response Status(const int &status)
        {
            this->setStatus(status, this->defaultStatusMessage(status));
            return *this;
        }

        void Send(std::string content)
        {
            this->body = content;
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

    private:
        std::string defaultStatusMessage(int code)
        {
            switch (code)
            {
            case 200:
                return "OK";
            case 201:
                return "Created";
            case 204:
                return "No Content";
            case 400:
                return "Bad Request";
            case 401:
                return "Unauthorized";
            case 403:
                return "Forbidden";
            case 404:
                return "Not Found";
            case 500:
                return "Internal Server Error";
            default:
                return "Unknown";
            }
        }
    };
}

#endif
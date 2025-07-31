#ifndef CORE_HTTP_REQUEST_RESPONSE_HPP
#define CORE_HTTP_REQUEST_RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <regex>

#include "ViewEngine/Engine.hpp"

namespace Http
{
    class Response
    {
    public:
        int statusCode = 200;
        std::string statusMessage = "OK";
        std::unordered_map<std::string, std::string> headers;
        std::string body;
        std::string viewDir = "./views";

        Nerva::TemplateEngine *_engine;

        void setStatus(int code, const std::string &message)
        {
            statusCode = code;
            statusMessage = message;
        }

        void setHeader(const std::string &key, const std::string &value)
        {
            headers[key] = value;
        }

        Response &operator<<(int code)
        {
            this->statusCode = code;
            return *this;
        }

        Response &operator<<(const std::string &str)
        {
            body += str;
            return *this;
        }

        void Render(const std::string view, Nerva::Context context)
        {
            std::string viewContent = _engine->render(view, context);
            setHeader("Content-Type", "text/html; charset=UTF-8");

            body = viewContent;
        }

        void MovedRedirect(std::string location)
        {
            body = "";

            statusCode = 301;
            statusMessage = "Moved Permanently";

            setHeader("Location", location);
        }

        void TemporaryRedirect(std::string location)
        {
            body = "";

            statusCode = 302;
            statusMessage = "Found";

            setHeader("Location", location);
        }

        void SendFile(std::string path);

        std::string detectContentType(const std::string &body) const
        {
            size_t start = body.find_first_not_of(" \t\n\r");
            if (start == std::string::npos)
                return "text/plain";

            if (body[start] == '{' || body[start] == '[')
                return "application/json";

            if (body.find("<html") != std::string::npos || body.find("<!DOCTYPE html") != std::string::npos)
                return "text/html";

            return "text/plain";
        }

        std::string toString() const
        {
            std::ostringstream responseStream;
            responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";

            if (headers.find("Content-Type") == headers.end())
            {
                responseStream << "Content-Type: " << detectContentType(body) << "\r\n";
            }

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
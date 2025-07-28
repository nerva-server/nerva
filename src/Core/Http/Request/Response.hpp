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

        Response status(const int &status)
        {
            this->setStatus(status, this->defaultStatusMessage(status));
            return *this;
        }

        Response &send(const std::string &content)
        {
            body = content;

            if (isJson(content))
            {
                setHeader("Content-Type", "application/json");
            }
            else if (isHtml(content))
            {
                setHeader("Content-Type", "text/html");
            }
            else
            {
                setHeader("Content-Type", "text/plain");
            }

            setHeader("Connection", "keep-alive");
            setHeader("Content-Length", std::to_string(body.size()));

            return *this;
        }

        std::string toString() const
        {
            std::ostringstream responseStream;
            responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
            for (const auto &[key, val] : headers)
            {
                responseStream << key << ": " << val << "\r\n";
            }
            responseStream << "\r\n"
                           << body;
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

        static bool isJson(const std::string &content)
        {
            return !content.empty() &&
                   ((content.front() == '{' && content.back() == '}') ||
                    (content.front() == '[' && content.back() == ']'));
        }

        static bool isHtml(const std::string &content)
        {
            static std::regex htmlRegex(R"(<\s*html[^>]*>)", std::regex_constants::icase);
            return std::regex_search(content, htmlRegex);
        }
    };
}

#endif
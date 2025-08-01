#ifndef CORE_HTTP_REQUEST_RESPONSE_HPP
#define CORE_HTTP_REQUEST_RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <ctime>
#include <iomanip>
#include <optional>
#include <chrono>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "Engine.hpp"

namespace Http
{
    struct CookieOptions
    {
        std::optional<std::chrono::seconds> maxAge;
        std::optional<std::string> path = "/";
        std::optional<std::string> domain;
        bool secure = false;
        bool httpOnly = false;
        std::optional<std::string> sameSite;
    };

    class Response
    {
    public:
        int statusCode = 200;
        std::string statusMessage = "OK";
        std::unordered_map<std::string, std::string> headers;
        std::string body;
        std::string viewDir = "./views";

        Nerva::TemplateEngine *_engine;
        std::unordered_map<std::string, std::string> incomingCookies;
        std::unordered_map<std::string, std::string> cookies;

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

        void Render(const std::string view, const nlohmann::json &context)
        {
            _engine->render(*this, view, context);
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

        Response &setCookie(const std::string &name,
                            const std::string &value,
                            const CookieOptions &options = {})
        {
            std::ostringstream cookie;
            cookie << name << "=" << value;

            if (options.maxAge)
            {
                cookie << "; Max-Age=" << options.maxAge->count();

                std::time_t expireTime = std::time(nullptr) + options.maxAge->count();
                std::tm tm = *std::gmtime(&expireTime);
                char buf[100];
                std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm);
                cookie << "; Expires=" << buf;
            }

            if (options.path)
                cookie << "; Path=" << *options.path;
            if (options.domain)
                cookie << "; Domain=" << *options.domain;
            if (options.secure)
                cookie << "; Secure";
            if (options.httpOnly)
                cookie << "; HttpOnly";
            if (options.sameSite)
                cookie << "; SameSite=" << *options.sameSite;

            cookies[name] = cookie.str();
            return *this;
        }

        std::optional<std::string> getCookie(const std::string &name) const
        {
            auto it = incomingCookies.find(name);
            return it != incomingCookies.end() ? std::make_optional(it->second) : std::nullopt;
        }

        std::string getCookieValue(const std::string &name,
                                   const std::string &defaultValue = "") const
        {
            auto cookie = getCookie(name);
            return cookie ? *cookie : defaultValue;
        }

        void removeCookie(const std::string &name,
                          const std::optional<std::string> &path = "/",
                          const std::optional<std::string> &domain = std::nullopt,
                          bool secure = false)
        {
            CookieOptions options;
            options.path = path;
            options.domain = domain;
            options.secure = secure;
            options.maxAge = std::chrono::seconds(-1);

            setCookie(name, "", options);
        }

        Response &setSignedCookie(const std::string &name,
                                  const std::string &value,
                                  const std::string &secret,
                                  const CookieOptions &options = {})
        {
            std::string signature = hmac_sha256(secret, value);
            return setCookie(name, value + "." + signature, options);
        }

        std::optional<std::string> getSignedCookie(const std::string &name,
                                                   const std::string &secret) const
        {
            auto cookie = getCookie(name);
            if (!cookie)
                return std::nullopt;

            size_t dot = cookie->rfind('.');
            if (dot == std::string::npos)
                return std::nullopt;

            std::string value = cookie->substr(0, dot);
            if (hmac_sha256(secret, value) != cookie->substr(dot + 1))
            {
                return std::nullopt;
            }
            return value.empty() ? "" : std::string(value);
        }

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

            for (const auto &[name, cookie] : cookies)
            {
                responseStream << "Set-Cookie: " << cookie << "\r\n";
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
        static std::string hmac_sha256(const std::string &key, const std::string &data)
        {
            unsigned char digest[EVP_MAX_MD_SIZE];
            unsigned int len;

            HMAC(EVP_sha256(),
                 key.c_str(), static_cast<int>(key.length()),
                 reinterpret_cast<const unsigned char *>(data.c_str()), data.length(),
                 digest, &len);

            std::ostringstream oss;
            for (unsigned int i = 0; i < len; i++)
            {
                oss << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(digest[i]);
            }
            return oss.str();
        }
    };
}

#endif
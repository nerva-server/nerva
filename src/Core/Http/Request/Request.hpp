#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

namespace Http
{
    class Request
    {
    public:
        Request() = default;

        bool parse(const std::string &rawRequest);

        std::string method;
        std::string path;
        std::string version;
        std::string body;
        std::map<std::string, std::string> headers;

    private:
        void parseHeaders(const std::string &headerString);
    };
}

#endif
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>

namespace Http
{
    class Request
    {
    public:
        Request() = default;

        std::string method;
        std::string path;
        std::string version;
        std::string body;
        std::map<std::string, std::string> headers;

        std::unordered_map<std::string, std::string> params;
        std::unordered_map<std::string, std::string> query;

        bool parse(const std::string &rawRequest)
        {
            std::istringstream stream(rawRequest);
            std::string requestLine;

            if (!std::getline(stream, requestLine))
            {
                return false;
            }

            std::istringstream lineStream(requestLine);
            if (!(lineStream >> method >> path >> version))
            {
                return false;
            }

            std::string headerLine;
            while (std::getline(stream, headerLine))
            {
                if (headerLine == "\r" || headerLine.empty())
                    break;

                size_t colonPos = headerLine.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string key = headerLine.substr(0, colonPos);
                    std::string value = headerLine.substr(colonPos + 1);

                    value.erase(0, value.find_first_not_of(" \t\r\n"));
                    value.erase(value.find_last_not_of(" \t\r\n") + 1);

                    headers[key] = value;
                }
            }

            body.assign(std::istreambuf_iterator<char>(stream),
                        std::istreambuf_iterator<char>());

            size_t queryPos = path.find('?');
            if (queryPos != std::string::npos)
            {
                std::string queryStr = path.substr(queryPos + 1);
                path = path.substr(0, queryPos);
                parseQueryString(queryStr);
            }

            return true;
        }
        
        const std::string &getParam(const std::string &key) const
        {
            static const std::string empty = "";
            auto it = params.find(key);
            return it != params.end() ? it->second : empty;
        }

        const std::string &getQuery(const std::string &key) const
        {
            static const std::string empty = "";
            auto it = query.find(key);
            return it != query.end() ? it->second : empty;
        }

        const std::string &getHeader(const std::string &key) const
        {
            static const std::string empty = "";
            auto it = headers.find(key);
            return it != headers.end() ? it->second : empty;
        }

    private:
        bool parseRouteParams(const std::string &routePattern)
        {
            return matchRouteAndExtractParams(routePattern);
        }

        void parseHeaders(const std::string &headerString)
        {
            std::istringstream stream(headerString);
            std::string line;
            while (std::getline(stream, line))
            {
                if (line.empty() || line == "\r")
                    continue;

                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string key = line.substr(0, colonPos);
                    std::string value = line.substr(colonPos + 1);
                    while (!value.empty() && (value.front() == ' ' || value.front() == '\r'))
                        value.erase(value.begin());
                    while (!value.empty() && (value.back() == ' ' || value.back() == '\r'))
                        value.pop_back();

                    headers[key] = value;
                }
            }
        }

        void parseQueryString(const std::string &queryString)
        {
            size_t start = 0;
            while (start < queryString.size())
            {
                size_t end = queryString.find('&', start);
                if (end == std::string::npos)
                    end = queryString.size();

                size_t eq = queryString.find('=', start);
                if (eq != std::string::npos && eq < end)
                {
                    std::string key = queryString.substr(start, eq - start);
                    std::string value = queryString.substr(eq + 1, end - eq - 1);
                    query[key] = value;
                }
                else
                {
                    std::string key = queryString.substr(start, end - start);
                    query[key] = "";
                }
                start = end + 1;
            }
        }

        std::vector<std::string> split(const std::string &str, char delim)
        {
            std::vector<std::string> parts;
            size_t start = 0;
            while (true)
            {
                size_t pos = str.find(delim, start);
                if (pos == std::string::npos)
                {
                    parts.push_back(str.substr(start));
                    break;
                }
                parts.push_back(str.substr(start, pos - start));
                start = pos + 1;
            }
            return parts;
        }

        bool matchRouteAndExtractParams(const std::string &routePattern)
        {
            auto routeParts = split(routePattern, '/');
            auto pathParts = split(path, '/');

            if (routeParts.size() != pathParts.size())
                return false;

            for (size_t i = 0; i < routeParts.size(); ++i)
            {
                if (!routeParts[i].empty() && routeParts[i][0] == ':')
                {
                    std::string key = routeParts[i].substr(1);
                    params[key] = pathParts[i];
                }
                else if (routeParts[i] != pathParts[i])
                {
                    return false;
                }
            }
            return true;
        }
    };
}

#endif

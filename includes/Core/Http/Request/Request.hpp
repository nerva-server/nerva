#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include "File.hpp"
#include <nlohmann/json.hpp>

namespace Http
{
    class Request
    {
    public:
        struct FormData
        {
            std::string value;
            File file;
            std::string filename;
            std::string contentType;
            bool isFile;
        };

        Request() = default;

        std::string method;
        std::string path;
        std::string version;
        std::vector<char> raw_data;
        std::map<std::string, std::string> headers;
        std::unordered_map<std::string, FormData> formData;
        nlohmann::json jsonBody;

        std::unordered_map<std::string, std::string> params;
        std::unordered_map<std::string, std::string> query;

        bool parse(const std::string &rawRequest);

        bool isMultipartFormData() const;
        bool isUrlEncodedFormData() const;
        bool isJsonData() const;
        
        const std::string &getParam(const std::string &key) const;
        const std::string &getQuery(const std::string &key) const;
        const std::string &getHeader(const std::string &key) const;
        const nlohmann::json &getJson() const;
        const FormData &getFormData(const std::string &key) const;

        bool hasParam(const std::string &key) const;
        bool hasQuery(const std::string &key) const;
        bool hasHeader(const std::string &key) const;
        bool hasFormData(const std::string &key) const;
        bool hasJsonBody() const;

    private:
        bool parseMultipartFormData();
        bool parseFormDataPart(const std::string &headers, const std::vector<char> &content);
        void parseUrlEncodedFormData();
        void parseJsonData();
        void parseQueryParameters();

        std::vector<std::string> split(const std::string &str, char delim);
        std::string urlDecode(const std::string &str);

        bool matchRouteAndExtractParams(const std::string &routePattern);

        bool has_json_body = false;
    };
}

#endif
#include "Request.hpp"

bool Http::Request::parse(const std::string &rawRequest)
{
    std::istringstream stream(rawRequest);
    std::string requestLine;

    if (!std::getline(stream, requestLine))
        return false;

    std::istringstream lineStream(requestLine);
    if (!(lineStream >> method >> path >> version))
        return false;

    std::string headerLine;
    while (std::getline(stream, headerLine))
    {
        if (headerLine.empty() || headerLine == "\r")
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

    raw_data.assign(std::istreambuf_iterator<char>(stream),
                    std::istreambuf_iterator<char>());

    if (isMultipartFormData())
    {
        if (!parseMultipartFormData())
            return false;
    }

    parseQueryParameters();

    return true;
}

bool Http::Request::isMultipartFormData() const
{
    auto it = headers.find("Content-Type");
    return it != headers.end() &&
           it->second.find("multipart/form-data") != std::string::npos;
}

const std::string &Http::Request::getParam(const std::string &key) const
{
    static const std::string empty = "";
    auto it = params.find(key);
    return it != params.end() ? it->second : empty;
}

const std::string &Http::Request::getQuery(const std::string &key) const
{
    static const std::string empty = "";
    auto it = query.find(key);
    return it != query.end() ? it->second : empty;
}

const std::string &Http::Request::getHeader(const std::string &key) const
{
    static const std::string empty = "";
    auto it = headers.find(key);
    return it != headers.end() ? it->second : empty;
}

const Http::Request::FormData &Http::Request::getFormData(const std::string &key) const
{
    static const FormData empty = {"", File(), "", "", false};
    auto it = formData.find(key);
    return it != formData.end() ? it->second : empty;
}

bool Http::Request::parseMultipartFormData()
{
    const std::string &contentType = headers.at("Content-Type");
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos)
        return false;

    std::string boundary = "--" + contentType.substr(boundaryPos + 9);
    std::string body(raw_data.begin(), raw_data.end());

    size_t pos = 0;
    while (pos < body.size())
    {
        size_t boundaryStart = body.find(boundary, pos);
        if (boundaryStart == std::string::npos)
            break;

        size_t partStart = boundaryStart + boundary.size();
        if (partStart >= body.size())
            break;

        if (body.substr(partStart, 2) == "--")
            break;

        partStart = body.find("\r\n", partStart);
        if (partStart == std::string::npos)
            break;
        partStart += 2;

        size_t headersEnd = body.find("\r\n\r\n", partStart);
        if (headersEnd == std::string::npos)
            break;

        std::string partHeaders = body.substr(partStart, headersEnd - partStart);
        size_t contentStart = headersEnd + 4;

        size_t partEnd = body.find(boundary, contentStart);
        if (partEnd == std::string::npos)
            partEnd = body.size();

        size_t contentEnd = (partEnd >= 2) ? partEnd - 2 : partEnd;
        if (contentEnd < contentStart)
            contentEnd = contentStart;

        std::vector<char> partContent(raw_data.begin() + contentStart,
                                      raw_data.begin() + contentEnd);

        if (!parseFormDataPart(partHeaders, partContent))
            return false;

        pos = partEnd;
    }

    return true;
}

bool Http::Request::parseFormDataPart(const std::string &headers, const std::vector<char> &content)
{
    size_t dispPos = headers.find("Content-Disposition:");
    if (dispPos == std::string::npos)
        return false;

    size_t namePos = headers.find("name=\"", dispPos);
    if (namePos == std::string::npos)
        return false;

    namePos += 6;
    size_t nameEnd = headers.find("\"", namePos);
    if (nameEnd == std::string::npos)
        return false;

    std::string name = headers.substr(namePos, nameEnd - namePos);
    FormData data;

    size_t filenamePos = headers.find("filename=\"", dispPos);
    if (filenamePos != std::string::npos)
    {
        filenamePos += 10;
        size_t filenameEnd = headers.find("\"", filenamePos);
        if (filenameEnd == std::string::npos)
            return false;

        data.filename = headers.substr(filenamePos, filenameEnd - filenamePos);
        data.isFile = true;

        size_t ctPos = headers.find("Content-Type:");
        if (ctPos != std::string::npos)
        {
            ctPos += 13;
            size_t ctEnd = headers.find("\r\n", ctPos);
            data.contentType = headers.substr(ctPos, ctEnd - ctPos);

            data.contentType.erase(0, data.contentType.find_first_not_of(" \t"));
            data.contentType.erase(data.contentType.find_last_not_of(" \t") + 1);
        }

        data.file = File(content);
    }
    else
    {
        data.value = std::string(content.begin(), content.end());
        data.isFile = false;
    }

    formData[name] = data;
    return true;
}

void Http::Request::parseQueryParameters()
{
    size_t queryPos = path.find('?');
    if (queryPos == std::string::npos)
        return;

    std::string queryStr = path.substr(queryPos + 1);
    path = path.substr(0, queryPos);

    size_t start = 0;
    while (start < queryStr.size())
    {
        size_t end = queryStr.find('&', start);
        if (end == std::string::npos)
            end = queryStr.size();

        size_t eq = queryStr.find('=', start);
        if (eq != std::string::npos && eq < end)
        {
            std::string key = queryStr.substr(start, eq - start);
            std::string value = queryStr.substr(eq + 1, end - eq - 1);
            query[key] = value;
        }
        else
        {
            std::string key = queryStr.substr(start, end - start);
            query[key] = "";
        }
        start = end + 1;
    }
}

std::vector<std::string> Http::Request::split(const std::string &str, char delim)
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

bool Http::Request::matchRouteAndExtractParams(const std::string &routePattern)
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
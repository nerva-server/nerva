#include "StaticFileHandler.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

StaticFileHandler::StaticFileHandler(const std::string &basePath) : basePath(basePath)
{
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "text/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".svg"] = "image/svg+xml";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".zip"] = "application/zip";
    mimeTypes[".mp3"] = "audio/mpeg";
    mimeTypes[".mp4"] = "video/mp4";
}

void StaticFileHandler::Handle(Http::Request &req, Http::Response &res, std::function<void()> next)
{
    if (req.method != "GET" && req.method != "HEAD")
    {
        next();
        return;
    }

    std::string filePath = resolvePath(req.path);

    if (!fileExists(filePath))
    {
        next();
        return;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        res << 403 << "Forbidden";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    res.headers["Content-Type"] = getMimeType(filePath);
    res.headers["Content-Length"] = std::to_string(content.size());

    if (req.method == "GET")
    {
        res << 200 << content;
    }
    else
    {
        res << 200;
    }
}


std::string StaticFileHandler::getMimeType(const std::string &path)
{
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        std::string ext = path.substr(dotPos);
        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end())
        {
            return it->second;
        }
    }
    return "application/octet-stream";
}

bool StaticFileHandler::SendFile(const std::string& filePath, Http::Response& res)
{
    struct stat buffer;
    if (stat(filePath.c_str(), &buffer) != 0 || !S_ISREG(buffer.st_mode))
    {
        res << 404 << "File not found";
        return false;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        res << 403 << "Forbidden";
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

    StaticFileHandler tempHandler("");
    std::string mimeType = tempHandler.getMimeType(filePath);

    res.headers["Content-Type"] = mimeType;
    res.headers["Content-Length"] = std::to_string(content.size());
    res << 200 << content;

    return true;
}

bool StaticFileHandler::fileExists(const std::string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

std::string StaticFileHandler::resolvePath(const std::string &requestPath)
{
    std::string result = basePath;

    if (result.back() != '/')
    {
        result += '/';
    }

    std::string cleanRequestPath = (requestPath.front() == '/') ? requestPath.substr(1) : requestPath;

    result += cleanRequestPath;

    if (result.back() == '/')
    {
        result += "index.html";
    }

    return result;
}
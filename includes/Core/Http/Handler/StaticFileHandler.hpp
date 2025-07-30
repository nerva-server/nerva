#ifndef STATIC_FILE_HANDLER_HPP
#define STATIC_FILE_HANDLER_HPP

#include <fstream>
#include <sys/stat.h>
#include <unordered_map>

#include "IHandler.hpp"

class StaticFileHandler : public IHandler
{
public:
    StaticFileHandler(const std::string &basePath);

    virtual void Handle(Http::Request &req, Http::Response &res, std::function<void()> next) override;

    static bool SendFile(const std::string& filePath, Http::Response& res);
    
private:
    std::string basePath;
    std::unordered_map<std::string, std::string> mimeTypes;

    std::string getMimeType(const std::string &path);
    bool fileExists(const std::string &path);
    std::string resolvePath(const std::string &requestPath);
};

#endif
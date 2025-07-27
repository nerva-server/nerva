#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request
{
public:
    Request();
    
    bool parse(const std::string &rawRequest);

    std::string getMethod() const;
    std::string getPath() const;
    std::string getVersion() const;
    std::string getHeader(const std::string &name) const;

private:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    
    void parseHeaders(const std::string &headerString);
};

#endif 

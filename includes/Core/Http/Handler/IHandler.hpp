#ifndef IHandler_hpp
#define IHandler_hpp

#include <string>
#include <functional>
#include <sstream>

#include "Core/Http/Request/Request.hpp"
#include "Core/Http/Request/Response.hpp"

class IHandler
{
public:
    virtual ~IHandler() = default;
    virtual void Handle(Http::Request &req, Http::Response &res, std::function<void()> next) = 0;
};

#endif
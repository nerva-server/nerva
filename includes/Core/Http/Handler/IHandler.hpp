#ifndef IHandler_hpp
#define IHandler_hpp

#include <string>
#include <functional>
#include <sstream>

#include "Request.hpp"
#include "Response.hpp"

class IHandler
{
public:
    virtual ~IHandler() = default;
    virtual void Handle(Http::Request &req, Http::Response &res, std::function<void()> next) = 0;
};

#endif
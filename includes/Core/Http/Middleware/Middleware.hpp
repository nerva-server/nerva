#ifndef NERVA_CORE_HTTP_MIDDLEWARE_MIDDLEWARE_HPP
#define NERVA_CORE_HTTP_MIDDLEWARE_MIDDLEWARE_HPP

#include <string>
#include <functional>

#include "IHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Middleware : public IHandler
{
    std::function<void(Http::Request &req, Http::Response &res, std::function<void()> next)> handler;

public:
    Middleware(std::function<void(Http::Request &req, Http::Response &res, std::function<void()> next)> handler) : handler(handler) {};

    virtual void Handle(Http::Request &req, Http::Response &res, std::function<void()> next)
    {
        if (handler)
        {
            handler(req, res, next);
        }
        else
        {
            next();
        }
    }
};

#endif
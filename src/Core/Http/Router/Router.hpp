#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <functional>
#include <map>

#include "Utils/Handlers.hpp"

class Router
{
public:
    Router();

    void addRoute(const std::string& method, const std::string &path, RequestHandler handler);

    void Get(const std::string &path, const RequestHandler &handler);
    void Post(const std::string &path, const RequestHandler &handler);

    std::string dispatch(const std::string &requestPath) const;

private:
    std::map<std::string, RequestHandler> routes;
};

#endif

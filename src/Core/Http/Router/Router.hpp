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

    std::string get(const std::string &requestPath, const RequestHandler &handler){
        addRoute("GET", requestPath, handler);
    };

    std::string post(const std::string &requestPath, const RequestHandler &handler){
        addRoute("POST", requestPath, handler);
    };

    std::string dispatch(const std::string &requestPath) const;

private:
    std::map<std::string, RequestHandler> routes;
};

#endif

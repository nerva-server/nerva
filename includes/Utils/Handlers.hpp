#ifndef HANDLERS_HPP
#define HANDLERS_HPP

#include <string>
#include <map>
#include <functional>

#include "Request.hpp"
#include "Response.hpp"

class Router;

using NextFunction = std::function<void()>;
using RequestHandler = std::function<void(const Http::Request&, Http::Response&, NextFunction)>;
using GroupHandler = std::function<void(Router&)>;

#endif
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <functional>

#include "Core/Http/Request/Request.hpp"
#include "Core/Http/Request/Response.hpp"

using RequestHandler = std::function<void(const Http::Request&, Http::Response&)>;

#endif
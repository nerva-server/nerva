#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <functional>

using RequestHandler = std::function<std::string(const std::string &requestPath)>;

#endif
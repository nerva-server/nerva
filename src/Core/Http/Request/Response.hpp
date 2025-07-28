#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

#include "Core/Http/Router/Router.hpp"

class Response
{
public:
    Response();
    
    bool parse(const std::string &rawRequest);

private:
    Router router;
};

#endif 

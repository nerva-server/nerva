#include "Core/Http/Request/Response.hpp"
#include "Core/Http/Handler/StaticFileHandler.hpp"

void Http::Response::SendFile(std::string path)
{
    ::StaticFileHandler::SendFile(path, *this);
}

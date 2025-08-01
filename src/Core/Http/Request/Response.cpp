#include "Response.hpp"
#include "StaticFileHandler.hpp"

void Http::Response::SendFile(std::string path)
{
    ::StaticFileHandler::SendFile(path, *this);
}

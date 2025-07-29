#include "RadixNode.hpp"
#include <sstream>
#include <utility>
#include <algorithm>
#include <stdexcept>

#include "Core/Http/Handler/IHandler.hpp"

RadixNode::RadixNode(std::string segment) : segment(std::move(segment)) {}
RadixNode::~RadixNode() = default;

void RadixNode::insert(const std::vector<std::reference_wrapper<IHandler>> &middlewares,
                       const std::string &method,
                       const std::string &path,
                       const RequestHandler &handler)
{
    auto segments = split(path);
    RadixNode *current = this;

    for (const auto &seg : segments)
    {
        RadixNode *child = current->findChild(seg);
        if (!child)
        {
            current->children.push_back(std::make_unique<RadixNode>(seg));
            child = current->children.back().get();
        }
        current = child;
    }

    current->methodHandlers[method] = handler;
    if (!middlewares.empty())
    {
        current->methodMiddlewares[method] = middlewares;
    }
}

std::optional<std::pair<RequestHandler, std::vector<std::reference_wrapper<IHandler>>>> RadixNode::find(const std::string &method, const std::string &path, std::map<std::string, std::string> &params) const
{
    auto segments = split(path);
    const RadixNode *current = this;

    for (const auto &seg : segments)
    {
        const RadixNode *next = current->findChild(seg);
        if (!next)
        {
            next = current->findParamChild();
            if (!next)
                return std::nullopt;

            std::string paramName = next->segment.substr(1);
            params[paramName] = seg;
        }
        current = next;
    }

    auto handlerIt = current->methodHandlers.find(method);
    if (handlerIt != current->methodHandlers.end())
    {
        std::vector<std::reference_wrapper<IHandler>> middlewares;
        auto mwIt = current->methodMiddlewares.find(method);
        if (mwIt != current->methodMiddlewares.end())
        {
            middlewares = mwIt->second;
        }

        return std::make_pair(handlerIt->second, middlewares);
    }

    return std::nullopt;
}

bool RadixNode::isParam() const
{
    return !segment.empty() && segment[0] == ':';
}

RadixNode *RadixNode::findChild(const std::string &seg) const
{
    for (const auto &child : children)
    {
        if (child->segment == seg && !child->isParam())
        {
            return child.get();
        }
    }
    return nullptr;
}

RadixNode *RadixNode::findParamChild() const
{
    for (const auto &child : children)
    {
        if (child->isParam())
            return child.get();
    }
    return nullptr;
}

std::vector<std::string> RadixNode::split(const std::string &path)
{
    std::vector<std::string> segments;
    std::stringstream ss(path);
    std::string item;

    while (std::getline(ss, item, '/'))
    {
        if (!item.empty())
        {
            segments.push_back(item);
        }
    }

    return segments;
}

#include "RadixNode.hpp"
#include <sstream>

RadixNode::RadixNode(std::string segment) : segment(std::move(segment)) {}
RadixNode::~RadixNode() = default;

void RadixNode::insert(const std::string &method, const std::string &path, Handler handler)
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
}

std::optional<Handler> RadixNode::find(const std::string &method, const std::string &path, std::map<std::string, std::string> &params) const
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

    auto it = current->methodHandlers.find(method);
    if (it != current->methodHandlers.end())
    {
        return it->second;
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

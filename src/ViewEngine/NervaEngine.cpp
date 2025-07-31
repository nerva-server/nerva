#include "ViewEngine/NervaEngine.hpp"

#include <sstream>

namespace Nerva
{
    std::shared_ptr<Value> createValue(bool value)
    {
        return std::make_shared<BoolValue>(value);
    }

    std::shared_ptr<Value> createValue(double value)
    {
        return std::make_shared<NumberValue>(value);
    }

    std::shared_ptr<Value> createValue(const std::string &value)
    {
        return std::make_shared<StringValue>(value);
    }

    std::shared_ptr<Value> createValue(const char *value)
    {
        return std::make_shared<StringValue>(value);
    }

    std::shared_ptr<ObjectValue> createObject(const std::map<std::string, std::shared_ptr<Value>> &value)
    {
        return std::make_shared<ObjectValue>(value);
    }

    std::shared_ptr<Value> createArray(const std::vector<std::map<std::string, std::shared_ptr<Value>>> &value)
    {
        std::vector<std::shared_ptr<Value>> items;
        for (const auto &obj : value)
        {
            items.push_back(createObject(obj));
        }
        return std::make_shared<ArrayValue>(items);
    }

    std::shared_ptr<Value> createArray(const std::vector<std::string> &value)
    {
        std::vector<std::shared_ptr<Value>> items;
        for (const auto &str : value)
        {
            items.push_back(createValue(str));
        }
        return std::make_shared<ArrayValue>(items);
    }

    std::shared_ptr<Context> createContext(const std::map<std::string, std::shared_ptr<Value>> &data)
    {
        return std::make_shared<Context>(data);
    }

}
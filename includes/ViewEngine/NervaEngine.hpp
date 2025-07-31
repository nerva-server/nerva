#ifndef NERVA_ENGINE_HPP
#define NERVA_ENGINE_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>

namespace Nerva
{

    class Value
    {
    public:
        virtual ~Value() = default;

        virtual bool isBool() const { return false; }
        virtual bool isNumber() const { return false; }
        virtual bool isString() const { return false; }
        virtual bool isObject() const { return false; }
        virtual bool isArray() const { return false; }

        virtual bool getBool() const { throw std::runtime_error("Not a boolean"); }
        virtual double getNumber() const { throw std::runtime_error("Not a number"); }
        virtual std::string getString() const { throw std::runtime_error("Not a string"); }
        virtual const std::map<std::string, std::shared_ptr<Value>> &getObject() const { throw std::runtime_error("Not an object"); }
        virtual const std::vector<std::shared_ptr<Value>> &getArray() const { throw std::runtime_error("Not an array"); }
    };

    class BoolValue : public Value
    {
        bool value;

    public:
        BoolValue(bool v) : value(v) {}
        bool isBool() const override { return true; }
        bool getBool() const override { return value; }
    };

    class NumberValue : public Value
    {
        double value;

    public:
        NumberValue(double v) : value(v) {}
        bool isNumber() const override { return true; }
        double getNumber() const override { return value; }
    };

    class StringValue : public Value
    {
        std::string value;

    public:
        StringValue(const std::string &v) : value(v) {}
        bool isString() const override { return true; }
        std::string getString() const override { return value; }
    };

    class ObjectValue : public Value
    {
        std::map<std::string, std::shared_ptr<Value>> value;

    public:
        ObjectValue(const std::map<std::string, std::shared_ptr<Value>> &v) : value(v) {}
        bool isObject() const override { return true; }
        const std::map<std::string, std::shared_ptr<Value>> &getObject() const override { return value; }
    };

    class ArrayValue : public Value
    {
        std::vector<std::shared_ptr<Value>> value;

    public:
        ArrayValue(const std::vector<std::shared_ptr<Value>> &v) : value(v) {}
        bool isArray() const override { return true; }
        const std::vector<std::shared_ptr<Value>> &getArray() const override { return value; }
    };

    class Context
    {
        std::map<std::string, std::shared_ptr<Value>> data;

    public:
        Context(const std::map<std::string, std::shared_ptr<Value>> &d) : data(d) {}
        const std::map<std::string, std::shared_ptr<Value>> &getData() const { return data; }
    };

    std::shared_ptr<Value> createValue(bool value);
    std::shared_ptr<Value> createValue(double value);
    std::shared_ptr<Value> createValue(const std::string &value);
    std::shared_ptr<Value> createValue(const char *value);
    std::shared_ptr<ObjectValue> createObject(const std::map<std::string, std::shared_ptr<Value>> &value);
    std::shared_ptr<Value> createArray(const std::vector<std::map<std::string, std::shared_ptr<Value>>> &value);
    std::shared_ptr<Value> createArray(const std::vector<std::string> &value);
    std::shared_ptr<Context> createContext(const std::map<std::string, std::shared_ptr<Value>> &data);

    class TemplateEngine
    {
    public:
        virtual ~TemplateEngine() = default;
        virtual std::string render(const std::string &templateName, const Context &context) = 0;
    };

}

#endif
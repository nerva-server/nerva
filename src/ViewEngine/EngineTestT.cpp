#include "ViewEngine/EngineTestT.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stack>
#include <cstring>
#include <cmath>
#include <iostream>

namespace Nerva
{

    void EngineTestT::setViewsDirectory(const std::string &path)
    {
        viewsDir = std::filesystem::path(path);
        if (!std::filesystem::exists(viewsDir))
        {
            throw std::runtime_error("Views directory does not exist: " + path);
        }
    }

    std::string EngineTestT::render(const std::string &templateName, const Context &context)
    {
        std::string templateContent = loadTemplate(templateName);
        return renderString(templateContent, context);
    }

    std::string EngineTestT::loadTemplate(const std::string &templateName)
    {
        auto it = templateCache.find(templateName);
        if (it != templateCache.end())
        {
            return it->second;
        }

        std::filesystem::path templatePath = viewsDir / (templateName + ".html");

        std::ifstream file(templatePath);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open template file: " + templatePath.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        templateCache[templateName] = content;
        return content;
    }

    std::string EngineTestT::renderString(const std::string &content, const Context &context)
    {
        std::string result;
        size_t pos = 0;
        size_t start_pos, end_pos;

        while ((start_pos = content.find("{{", pos)) != std::string::npos)
        {
            result += content.substr(pos, start_pos - pos);

            end_pos = content.find("}}", start_pos);
            if (end_pos == std::string::npos)
                break;

            std::string expr = content.substr(start_pos + 2, end_pos - start_pos - 2);
            expr.erase(0, expr.find_first_not_of(" \t\n\r"));
            expr.erase(expr.find_last_not_of(" \t\n\r") + 1);

            if (expr.rfind("for", 0) == 0)
            {
                size_t loopEnd = content.find("{{ endfor }}", end_pos);
                if (loopEnd == std::string::npos)
                {
                    pos = end_pos + 2;
                    continue;
                }

                std::string loopContent = content.substr(end_pos + 2, loopEnd - (end_pos + 2));
                std::string loopResult = processForLoop(expr, loopContent, context);
                result += loopResult;

                pos = loopEnd + strlen("{{ endfor }}");
            }
            else if (expr.rfind("if", 0) == 0)
            {
                size_t ifEnd = content.find("{{ endif }}", end_pos);
                if (ifEnd == std::string::npos)
                {
                    pos = end_pos + 2;
                    continue;
                }

                std::string ifContent = content.substr(end_pos + 2, ifEnd - (end_pos + 2));
                std::string ifResult = processIfCondition(expr, ifContent, context);
                result += ifResult;

                pos = ifEnd + strlen("{{ endif }}");
            }
            else if (expr.rfind("include", 0) == 0)
            {
                size_t withPos = expr.find(" with ");
                if (withPos != std::string::npos)
                {
                    std::string templateName = expr.substr(8, withPos - 8);
                    std::string contextVar = expr.substr(withPos + 6);

                    auto includeContext = resolvePath(contextVar, context.getData());
                    if (includeContext && includeContext->isObject())
                    {
                        auto newData = context.getData();
                        newData["it"] = includeContext;
                        Context newContext(newData);

                        std::string included = loadTemplate(templateName);
                        result += renderString(included, newContext);
                    }
                }
                else
                {
                    result += evaluateExpression(expr, context);
                }
                pos = end_pos + 2;
            }
            else
            {
                result += evaluateExpression(expr, context);
                pos = end_pos + 2;
            }
        }

        result += content.substr(pos);
        return result;
    }

    std::string EngineTestT::processForLoop(const std::string &loopExpr,
                                                     const std::string &loopContent,
                                                     const Context &context)
    {
        size_t inPos = loopExpr.find(" in ");
        if (inPos == std::string::npos)
            return "";

        std::string varsPart = loopExpr.substr(3, inPos - 3);
        std::string collectionName = loopExpr.substr(inPos + 4);

        varsPart.erase(0, varsPart.find_first_not_of(" \t"));
        varsPart.erase(varsPart.find_last_not_of(" \t") + 1);
        collectionName.erase(0, collectionName.find_first_not_of(" \t"));
        collectionName.erase(collectionName.find_last_not_of(" \t") + 1);

        auto collection = resolvePath(collectionName, context.getData());
        if (!collection || !collection->isArray())
            return "";

        std::string result;
        const auto &items = collection->getArray();

        bool hasIndex = varsPart.find(',') != std::string::npos;
        std::string itemVar, indexVar;

        if (hasIndex)
        {
            size_t commaPos = varsPart.find(',');
            itemVar = varsPart.substr(0, commaPos);
            indexVar = varsPart.substr(commaPos + 1);

            itemVar.erase(0, itemVar.find_first_not_of(" \t"));
            itemVar.erase(itemVar.find_last_not_of(" \t") + 1);
            indexVar.erase(0, indexVar.find_first_not_of(" \t"));
            indexVar.erase(indexVar.find_last_not_of(" \t") + 1);
        }
        else
        {
            itemVar = varsPart;
        }

        for (size_t i = 0; i < items.size(); i++)
        {
            auto newData = context.getData();
            newData[itemVar] = items[i];

            if (hasIndex)
            {
                newData[indexVar] = createValue((double)i);
            }

            Context newContext(newData);
            result += renderString(loopContent, newContext);
        }

        return result;
    }

    std::string EngineTestT::processIfCondition(const std::string &ifExpr,
                                                         const std::string &ifContent,
                                                         const Context &context)
    {
        size_t condStart = ifExpr.find_first_not_of(" \t", 2);
        if (condStart == std::string::npos)
            return "";

        std::string condition = ifExpr.substr(condStart);
        auto value = resolvePath(condition, context.getData());

        if (value && ((value->isBool() && value->getBool()) ||
                      (value->isNumber() && value->getNumber() != 0) ||
                      (value->isString() && !value->getString().empty())))
        {
            return renderString(ifContent, context);
        }

        return "";
    }

    std::string EngineTestT::evaluateExpression(const std::string &expr, const Context &context)
    {
        if (expr.rfind("include", 0) == 0)
        {
            size_t quotePos = expr.find('\'');
            if (quotePos == std::string::npos)
                quotePos = expr.find('"');
            if (quotePos == std::string::npos)
                return "";

            size_t endQuotePos = expr.find(expr[quotePos], quotePos + 1);
            if (endQuotePos == std::string::npos)
                return "";

            std::string templateName = expr.substr(quotePos + 1, endQuotePos - quotePos - 1);
            return loadTemplate(templateName);
        }
        else if (expr.find('|') != std::string::npos)
        {
            size_t pipePos = expr.find('|');
            std::string varName = expr.substr(0, pipePos);
            varName.erase(0, varName.find_first_not_of(" \t"));
            varName.erase(varName.find_last_not_of(" \t") + 1);

            std::string filter = expr.substr(pipePos + 1);
            filter.erase(0, filter.find_first_not_of(" \t"));
            filter.erase(filter.find_last_not_of(" \t") + 1);

            auto value = resolvePath(varName, context.getData());
            std::string result = getValueAsString(value);

            if (filter == "formatPrice")
            {
                size_t dotPos = result.find('.');
                if (dotPos != std::string::npos)
                {
                    result = result.substr(0, dotPos + 3);
                }
                if (result.length() > 6)
                {
                    result.insert(result.length() - 6, ".");
                }
            }
            else if (filter == "add:1")
            {
                try
                {
                    int num = std::stoi(result);
                    result = std::to_string(num + 1);
                }
                catch (...)
                {
                }
            }

            return result;
        }
        else
        {
            auto value = resolvePath(expr, context.getData());
            return getValueAsString(value);
        }
    }

    std::shared_ptr<Value> EngineTestT::resolvePath(const std::string &path, const std::map<std::string, std::shared_ptr<Value>> &data)
    {
        size_t dotPos = path.find('.');
        if (dotPos == std::string::npos)
        {
            auto it = data.find(path);
            return it != data.end() ? it->second : createValue("");
        }

        std::string firstPart = path.substr(0, dotPos);
        auto it = data.find(firstPart);
        if (it == data.end() || !it->second->isObject())
        {
            return createValue("");
        }

        std::string remainingPath = path.substr(dotPos + 1);
        return resolvePath(remainingPath, it->second->getObject());
    }

    std::string EngineTestT::getValueAsString(const std::shared_ptr<Value> &value)
    {
        if (!value)
            return "";

        if (value->isBool())
        {
            return value->getBool() ? "true" : "false";
        }
        else if (value->isNumber())
        {
            double num = value->getNumber();
            if (num == floor(num))
            {
                return std::to_string((int)num);
            }
            return std::to_string(num);
        }
        else if (value->isString())
        {
            return value->getString();
        }
        else if (value->isObject())
        {
            return "[object]";
        }
        else if (value->isArray())
        {
            return "[array]";
        }

        return "";
    }

}
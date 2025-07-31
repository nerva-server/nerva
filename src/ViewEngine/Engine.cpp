#include "ViewEngine/Engine.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "Core/Http/Request/Response.hpp"

namespace Nerva
{
    void Engine::setViewsDirectory(const std::string &path)
    {
        viewsDir = std::filesystem::path(path);
        if (!std::filesystem::exists(viewsDir))
        {
            throw std::runtime_error("Views directory does not exist: " + path);
        }
    }

    void Engine::render(Http::Response &res, const std::string &templateName, const json &context)
    {
        std::string templateContent = loadTemplate(templateName);

        res.setHeader("Content-Type", "text/html; charset=UTF-8");
        res.body = renderString(templateContent, context);
    }

    std::string Engine::loadTemplate(const std::string &templateName)
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

    std::string Engine::renderString(const std::string &content, const json &context)
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

                    json includeContext = resolvePath(contextVar, context);
                    if (!includeContext.is_null())
                    {
                        json newContext = context;
                        newContext["it"] = includeContext;

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

    std::string Engine::processForLoop(const std::string &loopExpr,
                                       const std::string &loopContent,
                                       const json &context)
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

        json collection = resolvePath(collectionName, context);
        if (!collection.is_array())
            return "";

        std::string result;
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

        size_t i = 0;
        for (auto &item : collection)
        {
            json newContext = context;
            newContext[itemVar] = item;

            if (hasIndex)
            {
                newContext[indexVar] = i;
            }

            result += renderString(loopContent, newContext);
            i++;
        }

        return result;
    }

    std::string Engine::processIfCondition(const std::string &ifExpr,
                                           const std::string &ifContent,
                                           const json &context)
    {
        size_t condStart = ifExpr.find_first_not_of(" \t", 2);
        if (condStart == std::string::npos)
            return "";

        std::string condition = ifExpr.substr(condStart);
        json value = resolvePath(condition, context);

        if (!value.is_null() &&
            ((value.is_boolean() && value.get<bool>()) ||
             (value.is_number() && value.get<double>() != 0) ||
             (value.is_string() && !value.get<std::string>().empty())))
        {
            return renderString(ifContent, context);
        }

        return "";
    }

    std::string Engine::evaluateExpression(const std::string &expr, const json &context)
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

            json value = resolvePath(varName, context);
            std::string result;

            if (value.is_boolean())
            {
                result = value.get<bool>() ? "true" : "false";
            }
            else if (value.is_number())
            {
                result = std::to_string(value.get<double>());
            }
            else if (value.is_string())
            {
                result = value.get<std::string>();
            }

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
            json value = resolvePath(expr, context);

            if (value.is_null())
                return "";
            if (value.is_boolean())
                return value.get<bool>() ? "true" : "false";
            if (value.is_number())
                return std::to_string(value.get<double>());
            if (value.is_string())
                return value.get<std::string>();
            if (value.is_object())
                return "[object]";
            if (value.is_array())
                return "[array]";

            return "";
        }
    }

    json Engine::resolvePath(const std::string &path, const json &data)
    {
        size_t dotPos = path.find('.');
        if (dotPos == std::string::npos)
        {
            return data.contains(path) ? data[path] : json();
        }

        std::string firstPart = path.substr(0, dotPos);
        if (!data.contains(firstPart))
            return json();

        std::string remainingPath = path.substr(dotPos + 1);
        return resolvePath(remainingPath, data[firstPart]);
    }
}
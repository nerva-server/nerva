
#include "Secure/Config/ConfigParser.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

ConfigParser::ConfigParser(const std::string &filename)
{
    parseFile(filename);
}

void ConfigParser::parseFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line))
    {
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.empty())
            continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos)
        {
            delimiterPos = line.find(' ');
            if (delimiterPos == std::string::npos)
                continue;
        }

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        configValues[key] = value;
    }
}

int ConfigParser::getInt(const std::string &key, int defaultValue) const
{
    auto it = configValues.find(key);
    if (it == configValues.end())
        return defaultValue;

    try
    {
        return std::stoi(it->second);
    }
    catch (...)
    {
        return defaultValue;
    }
}

std::string ConfigParser::getString(const std::string &key, const std::string &defaultValue) const
{
    auto it = configValues.find(key);
    if (it == configValues.end())
        return defaultValue;
    return it->second;
}
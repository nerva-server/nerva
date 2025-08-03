#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <unordered_map>
#include <stdexcept>

class ConfigParser
{
public:
    ConfigParser(const std::string &filename);

    int getInt(const std::string &key, int defaultValue = 0) const;
    std::string getString(const std::string &key, const std::string &defaultValue = "") const;
    bool getBool(const std::string &key, bool defaultValue = false) const;

private:
    std::unordered_map<std::string, std::string> configValues;
    void parseFile(const std::string &filename);
};

#endif
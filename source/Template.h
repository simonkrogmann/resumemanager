#pragma once

#include <sstream>
#include <string>

namespace util
{
class File;
}

class TemplateData;

class Template
{
public:
    Template(TemplateData* data, const util::File& templateFile);
    ~Template();

    std::string result();

    std::string beginTag = "<@";
    std::string endTag = "@>";
    std::string loopDelimiter = "|";

private:
    bool matchTag(const std::string& tag);
    std::string collectUntil(const std::string& tag,
                             const bool& spaces = false);
    void jumpToMatchingTag(const std::string& begin, const std::string& end);

    bool parse();
    void parseLoop();

    TemplateData* m_data;
    std::string m_templateContent;
    unsigned int m_position = 0;
    std::stringstream m_output;
};

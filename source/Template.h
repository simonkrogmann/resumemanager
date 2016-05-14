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
    std::string collectToMatchingTag(const std::string& end,
                                     const std::string& begin = "",
                                     const bool& spaces = true);

    bool parse();
    void parseLoop();

    TemplateData* m_data;
    std::string m_templateContent;
    unsigned int m_position = 0;
    std::stringstream m_output;
};

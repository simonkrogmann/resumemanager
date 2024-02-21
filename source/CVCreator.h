#pragma once

#include <sstream>
#include <string>

namespace util
{
class File;
}

class CVData;

class CVCreator
{
public:
    CVCreator(CVData* data);
    ~CVCreator();

    std::string result();

    std::string beginTag = "<@";
    std::string endTag = "@>";
    std::string loopDelimiter = "|";

private:
    bool matchTag(const std::string& tag);
    std::string collectToMatchingTag(const std::string& end,
                                     const std::string& begin = "",
                                     const bool& spaces = true);

    void parse(int limit = -1);
    void parseLoop();

    CVData* m_data;
    std::string m_templateContent;
    unsigned int m_position = 0;
    std::stringstream m_output;
};

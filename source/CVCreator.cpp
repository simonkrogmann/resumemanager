#include "CVCreator.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <utilgpu/cpp/file.h>
#include <utilgpu/cpp/resource.h>
#include <utilgpu/cpp/str.h>

#include "CVData.h"

CVCreator::CVCreator(CVData* data) : m_data{data}, m_templateContent{}
{
    auto templatePath = m_data->value("template");

    const auto file = util::startsWith(templatePath, "builtin")
                          ? util::loadResource<resumemanager>(templatePath)
                          : util::File(templatePath);
    assert(file.exists());
    if (!file.exists())
    {
        std::cerr << "Input file '" << file.path << "' does not exist."
                  << std::endl;
        exit(2);
    }
    m_templateContent = file.content();
}

CVCreator::~CVCreator() {}

bool CVCreator::matchTag(const std::string& tag)
{
    auto match = m_templateContent.substr(m_position, tag.size()) == tag;
    if (match)
    {
        m_position += tag.size();
    }
    return match;
}

std::string CVCreator::collectToMatchingTag(const std::string& end,
                                            const std::string& begin,
                                            const bool& spaces)
{
    std::stringstream collected;
    auto counter = 1;
    while (counter > 0)
    {
        if (m_position >= m_templateContent.size())
        {
            std::cerr << "Expected " << end << " but reached end instead."
                      << std::endl;
            exit(6);
        }
        if (begin != "" && matchTag(begin))
        {
            ++counter;
        }
        else if (matchTag(end))
        {
            --counter;
        }
        else
        {
            if (m_templateContent[m_position] != ' ' || spaces)
                collected << m_templateContent[m_position];
            ++m_position;
        }
    }
    return collected.str();
}
void CVCreator::parse(int limit)
{
    if (limit == -1)
    {
        limit = m_templateContent.size();
    }
    while (m_position < limit)
    {
        if (matchTag(beginTag))
        {
            if (matchTag(loopDelimiter))
            {
                parseLoop();
            }
            else
            {
                const auto name = collectToMatchingTag(endTag, beginTag, false);
                m_output << m_data->value(name);
            }
        }
        else if (matchTag(endTag))
        {
            std::cerr << "Extra close tag found at "
                      << m_templateContent.substr(m_position, 10) << std::endl;
            exit(4);
        }
        else
        {
            m_output << m_templateContent[m_position];
            ++m_position;
        }
    }
}

void CVCreator::parseLoop()
{
    const auto name = collectToMatchingTag(loopDelimiter, "", false);
    m_data->pushTag(name);
    if (!m_data->hasNext())
    {
        // skip section entirely
        collectToMatchingTag(endTag, beginTag);
        m_data->advance();
        return;
    }
    m_output << collectToMatchingTag(loopDelimiter);
    const auto loopBegin = m_position;
    collectToMatchingTag(endTag, beginTag, false);
    const auto loopEnd = m_position;
    const auto loopSep2 = m_templateContent.rfind(loopDelimiter, loopEnd);
    const auto loopSep = m_templateContent.rfind(loopDelimiter, loopSep2 - 1);
    m_position = loopSep + 1;
    const auto sep = collectToMatchingTag(loopDelimiter);

    while (true)
    {
        m_position = loopBegin;
        parse(loopSep);
        if (!m_data->advance())
        {
            break;
        }
        m_output << sep;
    }
    m_position = loopEnd;
}

std::string CVCreator::result()
{
    parse();
    return m_output.str();
}

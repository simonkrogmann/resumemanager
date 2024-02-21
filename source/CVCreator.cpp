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
    std::string collected = "";
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
                collected += m_templateContent[m_position];
            ++m_position;
        }
    }
    return collected;
}
bool CVCreator::parse()
{
    while (m_position < m_templateContent.size())
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
            return false;
        }
        else
        {
            m_output << m_templateContent[m_position];
            ++m_position;
        }
    }
    return true;
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
        if (matchTag(loopDelimiter))
        {
            collectToMatchingTag(loopDelimiter);
        }
        return;
    }
    m_output << collectToMatchingTag(loopDelimiter);
    const auto loopBegin = m_position;
    while (true)
    {
        m_position = loopBegin;
        if (parse())
        {
            std::cerr << "Extra close tag found" << std::endl;
            exit(4);
        }
        std::string split = "";
        if (matchTag(loopDelimiter))
        {
            split = collectToMatchingTag(loopDelimiter);
        }
        if (!m_data->advance())
        {
            break;
        }
        m_output << split;
    }
}

std::string CVCreator::result()
{
    parse();
    if (m_position != m_templateContent.size())
    {
        std::cerr << "Extra close tag found" << std::endl;
        exit(4);
    }
    return m_output.str();
}

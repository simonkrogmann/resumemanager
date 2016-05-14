#include "Template.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include <utilgpu/cpp/file.h>

#include "TemplateData.h"

Template::Template(TemplateData* data, const util::File& templateFile)
    : m_data{data}, m_templateContent{}
{
    assert(templateFile.exists());
    m_templateContent = templateFile.content();
}

Template::~Template()
{
}

bool Template::matchTag(const std::string& tag)
{
    auto match = m_templateContent.substr(m_position, tag.size()) == tag;
    if (match)
    {
        m_position += tag.size();
    }
    return match;
}

std::string Template::collectUntil(const std::string& tag, const bool& spaces)
{
    std::string collected = "";
    while (!matchTag(tag))
    {
        if (m_templateContent[m_position] != ' ' || spaces)
            collected += m_templateContent[m_position];
        ++m_position;
        if (m_position >= m_templateContent.size())
        {
            std::cout << "Expected " << tag << " but reached end instead."
                      << std::endl;
            exit(6);
        }
    }
    return collected;
}

void Template::jumpToMatchingTag(const std::string& begin,
                                 const std::string& end)
{
    auto counter = 1;
    while (counter > 0)
    {
        if (matchTag(begin))
        {
            ++counter;
        }
        else if (matchTag(end))
        {
            --counter;
        }
        else
        {
            ++m_position;
        }
    }
}
bool Template::parse()
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
                const auto name = collectUntil(endTag);
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

void Template::parseLoop()
{
    const auto name = collectUntil(loopDelimiter);
    m_data->pushTag(name);
    if (m_data->empty())
    {
        // skip loop content
        jumpToMatchingTag(beginTag, endTag);
        m_data->next();
        return;
    }
    m_output << collectUntil(loopDelimiter, true);
    const auto loopBegin = m_position;
    while (true)
    {
        m_position = loopBegin;
        if (parse())
        {
            std::cout << "Extra close tag found" << std::endl;
            exit(4);
        }
        std::string split = "";
        if (matchTag(loopDelimiter))
        {
            split = collectUntil(loopDelimiter, true);
        }
        if (m_data->next())
        {
            m_output << split;
        }
        else
        {
            break;
        }
    }
}

std::string Template::result()
{
    parse();
    if (m_position != m_templateContent.size())
    {
        std::cout << "Extra close tag found" << std::endl;
        exit(4);
    }
    return m_output.str();
}

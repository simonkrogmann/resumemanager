#include "Template.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <utilgpu/cpp/file.h>

#include "TemplateData.h"

Template::Template(TemplateData* data, const util::File& templateFile)
    : m_data{data}, m_templateContent{}
{
    assert(templateFile.exists());
    m_templateContent = templateFile.content();
}

Template::~Template() {}

bool Template::matchTag(const std::string& tag)
{
    auto match = m_templateContent.substr(m_position, tag.size()) == tag;
    if (match)
    {
        m_position += tag.size();
    }
    return match;
}

std::string Template::collectToMatchingTag(const std::string& end,
                                           const std::string& begin,
                                           const bool& spaces)
{
    std::string collected = "";
    auto counter = 1;
    while (counter > 0)
    {
        if (m_position >= m_templateContent.size())
        {
            std::cout << "Expected " << end << " but reached end instead."
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

void Template::parseLoop()
{
    const auto name = collectToMatchingTag(loopDelimiter, "", false);
    m_data->pushTag(name);
    if (m_data->empty())
    {
        // skip section entirely
        collectToMatchingTag(endTag, beginTag);
        m_data->next();
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
            std::cout << "Extra close tag found" << std::endl;
            exit(4);
        }
        std::string split = "";
        if (matchTag(loopDelimiter))
        {
            split = collectToMatchingTag(loopDelimiter);
        }
        if (!m_data->next())
        {
            break;
        }
        m_output << split;
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

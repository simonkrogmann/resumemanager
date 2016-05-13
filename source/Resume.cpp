#include "Resume.h"

#include <algorithm>
#include <iostream>

#include <utilgpu/cpp/cfl.h>
#include <utilgpu/cpp/file.h>

Resume::Resume(const util::File& database, const util::File& resume)
    : m_valid{true}
{
    m_database = loadCFL(database);
    m_resume = loadCFL(resume);
}

Resume::~Resume()
{
}

std::unique_ptr<util::CFLNode> Resume::loadCFL(util::File file)
{
    if (!file.exists())
    {
        std::cout << file.path << " does not exist." << std::endl;
        m_valid = false;
        return nullptr;
    }
    auto database = util::parseCFL(file.path);
    if (!database->valid())
    {
        std::cout << "Error in " << file.path << std::endl;
        std::cout << database->message() << std::endl;
        m_valid = false;
    }
    return database;
}

std::string resolveIndex(const unsigned int& index,
                         const std::vector<std::string>& source,
                         const std::vector<std::string>& alternative,
                         const std::vector<std::string>& important,
                         const std::vector<std::string>& remove)
{
    auto isElement = [](auto element, auto vector) {
        return std::find(vector.begin(), vector.end(), element) != vector.end();
    };
    if (alternative.size() > 0)
    {
        if (alternative.size() <= index)
        {
            return "";
        }
        return alternative[index];
    }
    if (important.size() > 0 || remove.size() > 0)
    {
        if (index < important.size())
        {
            return important[index];
        }
        auto actualIndex = important.size();
        for (const auto& el : source)
        {
            if (!isElement(el, important) && !isElement(el, remove))
            {
                ++actualIndex;
            }
            if (actualIndex > index)
            {
                return el;
            }
        }
        return "";
    }
    if (source.size() <= index)
    {
        return "";
    }
    return source[index];
}

util::CFLNode* Resume::query(const std::string& tag) const
{
    auto target = m_database.get();
    auto alt_target = m_resume.get();
    for (const auto& loop : m_loops)
    {
        std::vector<std::string> source;
        std::vector<std::string> important;
        std::vector<std::string> remove;
        std::vector<std::string> alternative;
        for (const auto& candidate : (*target)[loop.tag]->children())
        {
            source.push_back(candidate->name());
        }

        if (alt_target != nullptr)
        {
            important = (*alt_target)[loop.tag + "-important"]->values();
            remove = (*alt_target)[loop.tag + "-remove"]->values();
            alt_target = (*alt_target)[loop.tag];
            alternative = alt_target->values();
        }

        const auto actual =
            resolveIndex(loop.index, source, alternative, important, remove);

        if (actual == "")
        {
            return nullptr;
        }
        target = (*(*target)[loop.tag])[actual];

        // advance in specific resume
        if (alt_target != nullptr && alt_target->children().size() > 0)
        {
            alt_target = (*alt_target)[actual];
        }
        else
        {
            alt_target = nullptr;
        }
    }

    if (tag == "")
        return target;

    return (*target)[tag];
}

void Resume::pushTag(const std::string& tag)
{
    // TODO check presence
    m_loops.push_back({tag});
}

bool Resume::next()
{
    m_loops.back().index += 1;
    auto node = query();
    if (node == nullptr)
    {
        m_loops.pop_back();
        return false;
    }
    return true;
}

std::string Resume::value(const std::string& tag) const
{
    auto node = query(tag);
    assert(node != nullptr);
    return node->value();
}

bool Resume::valid() const
{
    return m_valid;
}

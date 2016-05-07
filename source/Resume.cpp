#include "Resume.h"

#include <iostream>

#include <utilgpu/cpp/file.h>
#include <utilgpu/cpp/cfl.h>

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

util::CFLNode* Resume::query(const std::string& tag) const
{
    auto target = m_database.get();
    auto alt_target = m_resume.get();
    for (const auto& loop : m_loops)
    {
        const auto& candidates = (*target)[loop.tag]->children();
        if (alt_target != nullptr)
        {
            alt_target = (*alt_target)[loop.tag];
        }

        // list overridden in specific resume
        // choose node according to specific resume
        if (alt_target != nullptr && alt_target->values().size() > 0)
        {
            const auto values = alt_target->values();
            if (loop.index >= values.size())
            {
                return nullptr;
            }
            target = (*(*target)[loop.tag])[values[loop.index]];
            alt_target = nullptr;
            continue;
        }

        if (loop.index >= candidates.size())
        {
            return nullptr;
        }
        target = candidates[loop.index].get();
        assert(target != nullptr);

        // advance in specific resume
        if (alt_target != nullptr && alt_target->children().size() > 0)
        {
            const auto& children = alt_target->children();
            if (loop.index >= children.size())
            {
                alt_target = nullptr;
            }
            else
            {
                alt_target = children[loop.index].get();
            }
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

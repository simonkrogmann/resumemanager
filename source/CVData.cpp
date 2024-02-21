#include "CVData.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include <utilgpu/cpp/cfl.h>
#include <utilgpu/cpp/file.h>

CVData::CVData(const util::File& database, const util::File& specific)
    : m_valid{true}
{
    m_database = loadCFL(database);
    m_resume = loadCFL(specific);
    m_stack.push_back({m_database.get(), m_resume.get()});
}

CVData::~CVData() {}

std::unique_ptr<util::CFLNode> CVData::loadCFL(util::File file)
{
    if (!file.exists())
    {
        std::cerr << "Template file '" << file.path << "' does not exist."
                  << std::endl;
        m_valid = false;
        return nullptr;
    }
    auto database = util::parseCFL(file.path);
    if (!database->valid())
    {
        std::cerr << "Error in " << file.path << std::endl;
        std::cerr << database->message() << std::endl;
        m_valid = false;
    }
    return database;
}

// Moves important nodes to beginning and removes unimportant node
// but not in actual data, only in the vector 'order'
void resolve(Loop& loop)
{
    auto& [node, alt_node, _, order] = loop;
    if (node != nullptr)
    {
        std::ranges::transform(node->children(), std::back_inserter(order),
                               [](const auto& x) { return x->name(); });
    }
    if (alt_node == nullptr) return;
    auto result = alt_node->values();
    if (result.empty())
    {
        auto important = (*alt_node)["important"];
        auto remove = (*alt_node)["remove"];
        std::vector<std::string> skip;
        if (important != nullptr)
        {
            result = important->values();
            skip = important->values();
        }
        if (remove != nullptr)
        {
            const auto& val = remove->values();
            skip.insert(skip.end(), val.begin(), val.end());
        }
        std::ranges::copy_if(
            order, std::back_inserter(result),
            [&](const auto& x)
            { return std::ranges::find(skip, x) == skip.end(); });
    }

    for (const auto& v : result)
    {
        if (std::ranges::find(order, v) == order.end())
        {
            std::cerr << "'" << v << "' from resume is not in database"
                      << std::endl;
            exit(5);
        }
    }
    order = result;
    assert(!(order.size() > 0 && node == nullptr));
}

IterationMode CVData::getIterationMode() const
{
    auto& [node, alt_node, _, _2] = m_stack.back();
    if (m_stack.size() == 1)
    {
        return IterationMode::None;
    }
    if (node != nullptr && !node->children().empty())
    {
        return IterationMode::Children;
    }
    if (alt_node != nullptr && !alt_node->values().empty())
    {
        return IterationMode::AltValues;
    }
    if (node != nullptr && !node->values().empty())
    {
        return IterationMode::NormalValues;
    }
    return IterationMode::Children;
}

void CVData::pushTag(const std::string& tag)
{
    assert(!m_stack.empty());
    auto& [node, alt_node, _, _2] = m_stack.back();
    auto new_node = (*node)[tag];
    auto new_alt = (alt_node == nullptr) ? nullptr : (*alt_node)[tag];
    m_stack.push_back({new_node, new_alt});
    resolve(m_stack.back());
}

bool CVData::advance()
{
    assert(getIterationMode() != IterationMode::None);
    m_stack.back().index += 1;
    if (!hasNext())
    {
        m_stack.pop_back();
        return false;
    }
    return true;
}

bool CVData::hasNext() const
{
    auto mode = getIterationMode();
    assert(mode != IterationMode::None);
    auto& [node, alt, index, order] = m_stack.back();
    int count = 0;
    if (mode == IterationMode::Children)
        count = order.size();
    else if (mode == IterationMode::NormalValues)
        count = node->values().size();
    else if (mode == IterationMode::AltValues)
        count = alt->values().size();
    return index < count;
}

std::string CVData::path() const
{
    std::string result;
    for (const auto& loop : m_stack)
    {
        result += (loop.node == nullptr) ? "0" : loop.node->name() + " -> ";
    }
    return result;
}

void queryError(const std::string& path)
{
    std::cerr << path << " does not exist." << std::endl;
    exit(1);
}

std::string CVData::value(const std::string& tag) const
{
    auto mode = getIterationMode();
    auto& [node, alt_node, index, order] = m_stack.back();
    if (mode == IterationMode::AltValues)
    {
        return alt_node->values()[index];
    }
    else if (mode == IterationMode::NormalValues)
    {
        return node->values()[index];
    }
    else if (mode == IterationMode::None)
    {
        if (alt_node != nullptr)
        {
            auto ret = (*alt_node)[tag];
            if (ret != nullptr) return ret->value();
        }
        auto ret = (*node)[tag];
        if (ret == nullptr) queryError(path() + tag);
        return ret->value();
    }
    else if (mode == IterationMode::Children)
    {
        if (node == nullptr) queryError(path() + tag);
        auto ret = (*(*node)[order[index]])[tag];
        if (ret == nullptr) queryError(path() + tag);
        return ret->value();
    }
    assert(false);
}

bool CVData::valid() const
{
    return m_valid;
}

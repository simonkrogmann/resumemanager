#include "Resume.h"

#include <iostream>

#include <utilgpu/cpp/file.h>
#include <utilgpu/cpp/cfl.h>

Resume::Resume(const util::File& database, const util::File& resume)
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

void Resume::pushTag(const std::string& tag)
{
    // TODO check presence
    m_tags.push_back({tag});
}

bool Resume::next()
{
    return false;
}

bool Resume::value(const std::string& tag) const
{
    return "dummy";
}

bool Resume::valid() const
{
    return m_valid;
}

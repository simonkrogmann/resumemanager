#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace util
{
class File;
class CFLNode;
}  // namespace util

struct Loop
{
    std::string tag;
    unsigned int index = 0;
};

class TemplateData
{
public:
    TemplateData(const util::File& database, const util::File& specific);
    ~TemplateData();

    void pushTag(const std::string& tag);
    bool next();
    bool empty() const;
    std::string value(const std::string& tag) const;

    bool valid() const;

private:
    std::unique_ptr<util::CFLNode> loadCFL(util::File file);
    util::CFLNode* query(const std::string& tag = "") const;
    std::string path() const;

    std::unique_ptr<util::CFLNode> m_database;
    std::unique_ptr<util::CFLNode> m_resume;
    std::vector<Loop> m_loops;
    bool m_valid;
};

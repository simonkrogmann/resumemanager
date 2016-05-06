#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace util
{
class File;
class CFLNode;
}

struct Loop
{
    std::string tag;
    unsigned int index = 0;
};

class Resume
{
public:
    Resume(const util::File& database, const util::File& resume);
    ~Resume();

    void pushTag(const std::string& tag);
    bool next();
    bool value(const std::string& tag) const;

    bool valid() const;

private:
    std::unique_ptr<util::CFLNode> loadCFL(util::File file);

    std::unique_ptr<util::CFLNode> m_database;
    std::unique_ptr<util::CFLNode> m_resume;
    std::vector<Loop> m_tags;
    bool m_valid;
};

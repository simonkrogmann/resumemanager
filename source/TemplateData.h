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
    util::CFLNode* node;
    util::CFLNode* alt_node;
    unsigned int index = 0;
    std::vector<std::string> order;
};

enum class IterationMode
{
    None,
    AltValues,
    NormalValues,
    Children
};

class TemplateData
{
public:
    TemplateData(const util::File& database, const util::File& specific);
    ~TemplateData();

    void pushTag(const std::string& tag);
    bool advance();
    bool hasNext() const;
    std::string value(const std::string& tag) const;
    bool valid() const;

private:
    std::unique_ptr<util::CFLNode> loadCFL(util::File file);
    std::string path() const;
    IterationMode getIterationMode() const;

    std::unique_ptr<util::CFLNode> m_database;
    std::unique_ptr<util::CFLNode> m_resume;
    std::vector<Loop> m_stack;
    bool m_valid;
};

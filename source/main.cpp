#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include <utilgpu/qt/Config.h>
#include <utilgpu/cpp/cfl.h>
#include <utilgpu/cpp/file.h>

struct Loop
{
    unsigned int begin;
    std::string tag;
    unsigned int index = 0;
};

int main(int argc, char* argv[])
{
    std::string applicationName = "build-resume";
    util::Config config{"simonkrogmann", applicationName};
    config.setDefaults({
        {"database", ""}, {"resume", ""}, {"template", ""},
    });
    config.load(argc, argv);
    std::map<std::string, std::unique_ptr<util::CFLNode>> databases;
    for (const auto& file : {"database", " resume"})
    {
        const auto filename = config.value(file);
        if (!util::fileExists(filename))
        {
            std::cout << filename << " does not exist." << std::endl;
            exit(1);
        }
        auto database = util::parseCFL(filename);
        if (!database->valid())
        {
            std::cout << "Error in " << filename << std::endl;
            std::cout << database->message() << std::endl;
            exit(2);
        }
        databases[file] = std::move(database);
    }

    const util::File file{config.value("template")};
    if (!file.exists())
    {
        std::cout << file.name << " does not exist." << std::endl;
        exit(1);
    }
    auto templateContent = file.content();

    const std::string beginTag = "<@";
    const std::string endTag = "@>";
    const std::string loopDelimiter = "|";

    auto isTag = [&](unsigned int i, std::string tag)
    {
        return templateContent.substr(i, tag.size()) == tag;
    };
    auto collectUntil = [&](unsigned int& i, std::string tag)
    {
        std::string collected = "";
        while (isTag(i, tag))
        {
            if (templateContent[i] != ' ')
                collected += templateContent[i];
            ++i;
        }
        i += tag.size();
        return collected;
    };

    std::vector<Loop> loops;
    std::stringstream output;
    for (unsigned int i = 0; i < templateContent.size(); ++i)
    {
        if (isTag(i, beginTag))
        {
            i += beginTag.size();
            if (isTag(i, loopDelimiter))
            {
                i += loopDelimiter.size();
                auto name = collectUntil(i, loopDelimiter);
                loops.push_back({i, name});
            }
            else
            {
                auto name = collectUntil(i, endTag);
                // TODO insert replacement
            }
        }
        else if (isTag(i, endTag))
        {
            i += endTag.size();
            if (1 /* loop is finished */)
            {
                loops.pop_back();
            }
            else
            {
                loops.back().index += 1;
                i = loops.back().begin;
            }
        }
        else
        {
            output << templateContent[i];
        }
    }
    util::File out{config.value("resume") + ".tex"};
    out.setContent(output.str());
    std::string command = "pdflatex ";
    if (system((command + out.name).c_str()))
    {
        std::cout << "Error while building pdf." << std::endl;
        exit(3);
    }
    return 0;
}

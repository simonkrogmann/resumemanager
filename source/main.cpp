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
    for (const auto& file : {"database", "resume"})
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
    const auto templateContent = file.content();

    const std::string beginTag = "<@";
    const std::string endTag = "@>";
    const std::string loopDelimiter = "|";

    const auto isTag = [&](unsigned int& i, std::string tag)
    {
        auto match = templateContent.substr(i, tag.size()) == tag;
        if (match)
        {
            i += tag.size();
        }
        return match;
    };
    const auto collectUntil = [&](unsigned int& i, std::string tag)
    {
        std::string collected = "";
        while (!isTag(i, tag))
        {
            if (templateContent[i] != ' ')
                collected += templateContent[i];
            ++i;
            // TODO: error message on end
        }
        return collected;
    };

    std::vector<Loop> loops;
    std::stringstream output;
    for (unsigned int i = 0; i < templateContent.size();)
    {
        if (isTag(i, beginTag))
        {
            if (isTag(i, loopDelimiter))
            {
                const auto name = collectUntil(i, loopDelimiter);
                loops.push_back({i, name});
                // TODO: check tag existence
            }
            else
            {
                const auto name = collectUntil(i, endTag);
                // TODO: insert replacement
            }
        }
        else if (isTag(i, endTag))
        {
            if (1 /* TODO: loop is finished */)
            {
                assert(loops.size() > 0);
                // TODO: error message
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
            ++i;
        }
    }
    util::File out{config.value("resume") + ".tex"};
    out.setContent(output.str());
    std::string command = "pdflatex -interaction=batchmode ";
    if (system((command + out.path).c_str()))
    {
        std::cout << "Error while building pdf." << std::endl;
        exit(3);
    }
    return 0;
}

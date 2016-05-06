#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include <utilgpu/qt/Config.h>
#include <utilgpu/cpp/cfl.h>
#include <utilgpu/cpp/file.h>

#include "Resume.h"

int main(int argc, char* argv[])
{
    std::string applicationName = "build-resume";
    util::Config config{"simonkrogmann", applicationName};
    config.setDefaults({
        {"database", ""}, {"resume", ""}, {"template", ""},
    });
    config.load(argc, argv);

    Resume resume{config.value("database"), config.value("resume")};
    if (!resume.valid())
    {
        exit(1);
    }

    const util::File file{config.value("template")};
    if (!file.exists())
    {
        std::cout << file.name << " does not exist." << std::endl;
        exit(2);
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

    std::vector<unsigned int> loops;
    std::stringstream output;
    for (unsigned int i = 0; i < templateContent.size();)
    {
        if (isTag(i, beginTag))
        {
            if (isTag(i, loopDelimiter))
            {
                const auto name = collectUntil(i, loopDelimiter);
                loops.push_back(i);
                resume.pushTag(name);
            }
            else
            {
                const auto name = collectUntil(i, endTag);
                output << resume.value(name);
            }
        }
        else if (isTag(i, endTag))
        {
            if (resume.next())
            {
                i = loops.back();
            }
            else
            {
                assert(loops.size() > 0);
                // TODO: error message
                loops.pop_back();
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

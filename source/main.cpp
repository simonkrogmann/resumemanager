#include <iostream>
#include <map>
#include <string>

#include <utilgpu/cpp/file.h>
#include <utilgpu/cpp/resource.h>
#include <utilgpu/cpp/str.h>
#include <utilgpu/qt/Config.h>

#include "Template.h"
#include "TemplateData.h"

int main(int argc, char* argv[])
{
    std::string applicationName = "build-resume";
    util::Config config{"simonkrogmann", applicationName};
    config.setDefaults({
        {"database", ""}, {"resume", ""}, {"template", "builtin/base.tex"},
    });
    config.load(argc, argv);

    TemplateData resume{config.value("database"), config.value("resume")};
    if (!resume.valid())
    {
        exit(1);
    }

    const auto templatePath = config.value("template");

    const auto file = util::startsWith(templatePath, "builtin")
                          ? util::loadResource<resumemanager>(templatePath)
                          : util::File(templatePath);
    if (!file.exists())
    {
        std::cout << file.path << " does not exist." << std::endl;
        exit(2);
    }
    Template resumeTemplate{&resume, file};

    util::File out{config.value("resume") + ".tex"};
    out.setContent(resumeTemplate.result());
    std::string command = "pdflatex -interaction=batchmode ";
    if (system((command + out.path).c_str()))
    {
        std::cout << "Error while building pdf." << std::endl;
        exit(3);
    }
    return 0;
}

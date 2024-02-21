#include <iostream>
#include <string>

#include <utilgpu/cpp/file.h>

#include "CVCreator.h"
#include "CVData.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage:" << argv[0] << " <resume> <database>" << std::endl;
        exit(5);
    }
    std::string filename{argv[1]};
    std::string db_file{argv[2]};

    CVData resume{db_file, filename};
    if (!resume.valid())
    {
        exit(1);
    }

    CVCreator resumeTemplate{&resume};

    util::File out{filename + ".tex"};
    out.setContent(resumeTemplate.result());
    std::string command = "pdflatex -interaction=batchmode ";
    if (system((command + out.path).c_str()))
    {
        std::cerr << "Error while building pdf." << std::endl;
        exit(3);
    }
    return 0;
}

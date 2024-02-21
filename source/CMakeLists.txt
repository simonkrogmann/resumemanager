set(target build-resume)

set(sources
    main.cpp
    CVData.cpp
    CVCreator.cpp
)

set(includes
    CVData.h
    CVCreator.h
)

set(resources
    builtin/base.tex
    builtin/onecolumn.tex
)

set(libraries
    utilgpu::cpp
)

create_executable(${project} ${target} "${sources}" "${includes}" "${libraries}")
generate_resource_header(${project} ${target} "${resources}")

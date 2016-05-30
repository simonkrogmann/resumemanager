set(target build-resume)

set(sources
    main.cpp
    TemplateData.cpp
    Template.cpp
)

set(includes
    TemplateData.h
    Template.h
)

set(resources
    builtin/base.tex
    builtin/onecolumn.tex
)

set(libraries
    utilgpu::cpp utilgpu::qt
)

create_executable(${project} ${target} "${sources}" "${includes}" "${libraries}")
generate_resource_header(${project} ${target} "${resources}")

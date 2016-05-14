set(target build-resume)

set(sources
    main.cpp
    Resume.cpp
)

set(includes
    Resume.h
)

set(resources
    builtin/base.tex
)

set(libraries
    utilgpu::cpp utilgpu::qt
)

create_executable(${project} ${target} "${sources}" "${includes}" "${libraries}")
generate_resource_header(${project} ${target} "${resources}")

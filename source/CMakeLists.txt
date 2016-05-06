set(target build-resume)

set(sources
    main.cpp
    Resume.cpp
)

set(includes
    Resume.h
)

set(resources
)

set(libraries
    utilgpu::cpp utilgpu::qt
)

create_executable(${project} ${target} "${sources}" "${includes}" "${libraries}")

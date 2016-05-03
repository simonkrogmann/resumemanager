set(target build-resume)

set(sources
    main.cpp
)

set(includes
)

set(resources
)

set(libraries
    utilgpu::cpp utilgpu::qt
)

create_executable(${project} ${target} "${sources}" "${includes}" "${libraries}")

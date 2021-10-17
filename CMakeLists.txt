cmake_minimum_required(VERSION 3.2.3 FATAL_ERROR)

project(dumpling)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /W3 /permissive-")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:console")
else()
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

set(SOURCES
    data.c
    graphic.c
    main.c
    tigr.c
    util.c)

add_executable(dumpling ${SOURCES})

if(WIN32)
    target_link_libraries(dumpling PRIVATE opengl32.lib gdi32.lib)
elseif(APPLE)
    target_link_libraries(dumpling PRIVATE "-framework OpenGL" "-framework Cocoa")
else()
    target_link_libraries(dumpling PRIVATE GLU GL X11 SDL2)
endif()

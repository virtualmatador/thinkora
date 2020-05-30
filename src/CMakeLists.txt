cmake_minimum_required(VERSION 3.16)
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})
add_executable(thinkora
    bar.cpp
    board.cpp
    circle.cpp
    convex.cpp
    job.cpp
    line.cpp
    main.cpp
    ocr.cpp
    pattern.cpp
    point.cpp
    result.cpp
    shape.cpp
    sketch.cpp
    text.cpp
    toolbox.cpp
    window.cpp
)
set_property(TARGET thinkora PROPERTY CXX_STANDARD 20)
target_link_libraries(thinkora jsonio ${GTKMM_LIBRARIES} pthread)

cmake_minimum_required(VERSION 3.16)
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})
add_executable(thinkora
    bar.cpp
    board.cpp
    character.cpp
    circle.cpp
    convex.cpp
    dot.cpp
    guess.cpp
    main.cpp
    ocr.cpp
    shape.cpp
    sketch.cpp
    text.cpp
    toolbox.cpp
    window.cpp
    wire.cpp
)
add_dependencies("thinkora" "patterns")
set_property(TARGET thinkora PROPERTY CXX_STANDARD 20)
target_link_libraries(thinkora jsonio ${GTKMM_LIBRARIES} pthread)

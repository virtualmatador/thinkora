cmake_minimum_required(VERSION 3.16)
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-4.0)
find_program(JQ_EXECUTABLE jq)
if (NOT JQ_EXECUTABLE)
    message(FATAL_ERROR "jq is required to generate Thinkora data sources")
endif()

set(THINKORA_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(THINKORA_GENERATED_CHARACTERS_CPP
    "${THINKORA_GENERATED_DIR}/characters.cpp"
)
set(THINKORA_GENERATED_RENDERS_CPP
    "${THINKORA_GENERATED_DIR}/renders.cpp"
)
set(THINKORA_GENERATED_SHAPES_CPP
    "${THINKORA_GENERATED_DIR}/shapes.cpp"
)
set(THINKORA_JQ_GENERATOR
    "${CMAKE_SOURCE_DIR}/cmake/run_jq_generator.cmake"
)

file(GLOB THINKORA_CHARACTER_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/characters/*.json"
)
file(GLOB THINKORA_RENDER_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/render/*.json"
)
file(GLOB THINKORA_SHAPE_PATTERN_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/shapes/*.json"
)

add_custom_command(
    OUTPUT "${THINKORA_GENERATED_CHARACTERS_CPP}"
    COMMAND "${CMAKE_COMMAND}"
        -D "JQ_EXECUTABLE=${JQ_EXECUTABLE}"
        -D "JQ_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/generate_characters_cpp.jq"
        -D "INPUT_GLOB=${CMAKE_SOURCE_DIR}/characters/*.json"
        -D "OUTPUT=${THINKORA_GENERATED_CHARACTERS_CPP}"
        -P "${THINKORA_JQ_GENERATOR}"
    DEPENDS
        "${THINKORA_JQ_GENERATOR}"
        "${CMAKE_SOURCE_DIR}/cmake/generate_characters_cpp.jq"
        ${THINKORA_CHARACTER_FILES}
    VERBATIM
)

add_custom_command(
    OUTPUT "${THINKORA_GENERATED_RENDERS_CPP}"
    COMMAND "${CMAKE_COMMAND}"
        -D "JQ_EXECUTABLE=${JQ_EXECUTABLE}"
        -D "JQ_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/generate_renders_cpp.jq"
        -D "INPUT_GLOB=${CMAKE_SOURCE_DIR}/render/*.json"
        -D "OUTPUT=${THINKORA_GENERATED_RENDERS_CPP}"
        -P "${THINKORA_JQ_GENERATOR}"
    DEPENDS
        "${THINKORA_JQ_GENERATOR}"
        "${CMAKE_SOURCE_DIR}/cmake/generate_renders_cpp.jq"
        ${THINKORA_RENDER_FILES}
    VERBATIM
)

add_custom_command(
    OUTPUT "${THINKORA_GENERATED_SHAPES_CPP}"
    COMMAND "${CMAKE_COMMAND}"
        -D "JQ_EXECUTABLE=${JQ_EXECUTABLE}"
        -D "JQ_SCRIPT=${CMAKE_SOURCE_DIR}/cmake/generate_shapes_cpp.jq"
        -D "INPUT_GLOB=${CMAKE_SOURCE_DIR}/shapes/*.json"
        -D "OUTPUT=${THINKORA_GENERATED_SHAPES_CPP}"
        -P "${THINKORA_JQ_GENERATOR}"
    DEPENDS
        "${THINKORA_JQ_GENERATOR}"
        "${CMAKE_SOURCE_DIR}/cmake/generate_shapes_cpp.jq"
        ${THINKORA_SHAPE_PATTERN_FILES}
    VERBATIM
)

add_library(thinkora_core STATIC
    bar.cpp
    board.cpp
    character.cpp
    circle.cpp
    convex.cpp
    dot.cpp
    "${THINKORA_GENERATED_CHARACTERS_CPP}"
    guess.cpp
    ocr.cpp
    render.cpp
    "${THINKORA_GENERATED_RENDERS_CPP}"
    shape.cpp
    "${THINKORA_GENERATED_SHAPES_CPP}"
    sketch.cpp
    text.cpp
    toolbox.cpp
    window.cpp
    wire.cpp
)
set_property(TARGET thinkora_core PROPERTY CXX_STANDARD 20)
target_include_directories(thinkora_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTKMM_INCLUDE_DIRS}
)
target_link_directories(thinkora_core PUBLIC ${GTKMM_LIBRARY_DIRS})
target_link_libraries(thinkora_core PUBLIC ${GTKMM_LIBRARIES} pthread)

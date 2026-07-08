if (NOT DEFINED JQ_EXECUTABLE)
    message(FATAL_ERROR "JQ_EXECUTABLE is required")
endif()
if (NOT DEFINED JQ_SCRIPT)
    message(FATAL_ERROR "JQ_SCRIPT is required")
endif()
if (NOT DEFINED INPUT_GLOB)
    message(FATAL_ERROR "INPUT_GLOB is required")
endif()
if (NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif()

file(GLOB INPUTS "${INPUT_GLOB}")
list(SORT INPUTS)

get_filename_component(OUTPUT_DIR "${OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")

execute_process(
    COMMAND "${JQ_EXECUTABLE}" -n -r -f "${JQ_SCRIPT}" ${INPUTS}
    RESULT_VARIABLE JQ_RESULT
    ERROR_VARIABLE JQ_ERROR
    OUTPUT_FILE "${OUTPUT}"
)

if (NOT JQ_RESULT EQUAL 0)
    message(FATAL_ERROR "jq generation failed for ${OUTPUT}: ${JQ_ERROR}")
endif()

add_executable(ocr_alphabet
    ocr_alphabet.cpp
)
set_property(TARGET ocr_alphabet PROPERTY CXX_STANDARD 20)
target_link_libraries(ocr_alphabet PRIVATE thinkora_core)

add_test(NAME ocr_alphabet COMMAND ocr_alphabet)
set_tests_properties(ocr_alphabet PROPERTIES
    RUN_SERIAL TRUE
    SKIP_RETURN_CODE 77
    TIMEOUT 90
)

add_executable(text_rendering
    text_rendering.cpp
)
set_property(TARGET text_rendering PROPERTY CXX_STANDARD 20)
target_link_libraries(text_rendering PRIVATE thinkora_core)

add_test(NAME text_rendering COMMAND text_rendering)

add_executable(convex_invert
    convex_invert.cpp
)
set_property(TARGET convex_invert PROPERTY CXX_STANDARD 20)
target_link_libraries(convex_invert PRIVATE thinkora_core)

add_test(NAME convex_invert COMMAND convex_invert)

add_executable(ocr_shapes
    ocr_shapes.cpp
)
set_property(TARGET ocr_shapes PROPERTY CXX_STANDARD 20)
target_link_libraries(ocr_shapes PRIVATE thinkora_core)

add_test(NAME ocr_shapes COMMAND ocr_shapes)
set_tests_properties(ocr_shapes PROPERTIES
    RUN_SERIAL TRUE
    SKIP_RETURN_CODE 77
    TIMEOUT 90
)

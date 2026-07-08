add_executable(thinkora
    main.cpp
)
set_property(TARGET thinkora PROPERTY CXX_STANDARD 20)
target_link_libraries(thinkora PRIVATE thinkora_core)

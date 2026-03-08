include_directories(${PROJECT_SOURCE_DIR}/include)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
endif(UNIX)

add_executable(htd_min_degree_standalone
    main.cpp
    simple_graph_reader.cpp
)

set_property(TARGET htd_min_degree_standalone PROPERTY CXX_STANDARD 11)
set_property(TARGET htd_min_degree_standalone PROPERTY CXX_STANDARD_REQUIRED ON)

if(UNIX)
    set_target_properties(htd_min_degree_standalone
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif(UNIX)

install(TARGETS htd_min_degree_standalone DESTINATION bin)

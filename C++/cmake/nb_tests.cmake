# Provides a macro allowing NetBeans to build and run tests
# Adds target to call test script that execute tests

add_custom_target(build-tests)

add_custom_target(test
    DEPENDS build-tests
    COMMAND ./test ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

macro(ADD_NB_TEST EXECUTABLE SOURCE)
    add_executable(${EXECUTABLE} EXCLUDE_FROM_ALL ${SOURCE})
    target_link_libraries(${EXECUTABLE} ${LIBRARIES})
    add_dependencies(build-tests ${EXECUTABLE})
endmacro()
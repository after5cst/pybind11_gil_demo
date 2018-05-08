cmake_minimum_required(VERSION 3.2)

if (NOT PROJECT_NAME)
    message(FATAL_ERROR 
    "PROJECT_NAME not defined before including cmake_helpers"
    )
endif(NOT PROJECT_NAME)

if (NOT("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}"))
    message(FATAL_ERROR 
    "include(cmake_helpers) MUST ONLY be in root CMakeLists.txt"
    )
endif (NOT("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}"))

# Setup C++ compile settings, including warnings and minimum version.
include(${CMAKE_CURRENT_LIST_DIR}/SetupCppCompiler.cmake)

# Import the AddClangFormat() function
include(${CMAKE_CURRENT_LIST_DIR}/LocateProgram.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/AddClangFormat.cmake)

# Since Python requires shared objects, this flag needs to be on
# so that the compiler doesn't optimize jumps in a way that DLLs
# are "out of luck"
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add Python unit test support ('make test')
enable_testing()
include(${CMAKE_CURRENT_LIST_DIR}/AddPythonUnitTests.cmake)
AddPythonUnitTests( "${PROJECT_NAME}" "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}")
file(GLOB_RECURSE PYTHON_TEST_SCRIPTS ${PROJECT_SOURCE_DIR} test_*.py)

# pybind11 is our library that makes C++ to Python "easy"
# "easy" is very much a relative term.
add_subdirectory(${CMAKE_SOURCE_DIR}/pybind11)
pybind11_add_module("${PROJECT_NAME}" ${PYTHON_TEST_SCRIPTS})


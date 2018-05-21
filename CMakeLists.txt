cmake_minimum_required(VERSION 3.2)

#----------------------------------------------------------
# Step 1: Setup the project with a project name.
#         (Also configure the output as a pybind11 project)
#
set (PROJECT_NAME gild)
project(${PROJECT_NAME})
add_definitions("-g")
include(cmake_helpers/helpers.cmake)

#----------------------------------------------------------
# Step 2: Add this directory (root of the project) 
# to the include path.  This allows relative includes from
# base to be used.
include_directories(${CMAKE_CURRENT_LIST_DIR})

#----------------------------------------------------------
# Step 3: Add reusable sources and subdirectories.

include(worker/worker.cmake)

#----------------------------------------------------------
# Step 4: Add the module initialization file.  There can
#         be only one, so it's not inheritable by a parent
#         project (like items in step 2 can be)
target_sources( ${PROJECT_NAME}
    PUBLIC
    "init_module.cpp"
    "count.cpp"
    "count.h"
    # "enable_shared_from_this.h"
    "really_async.h"
    )

AddClangFormat(${PROJECT_NAME})


# Set current directory for adding sources
set (HERE ${CMAKE_CURRENT_LIST_DIR})

target_sources("${PROJECT_NAME}"
    PRIVATE
    "${HERE}/input.h"
    "${HERE}/job.cpp"
    "${HERE}/job.h"
    "${HERE}/launch.cpp"
    "${HERE}/launch.h"
    "${HERE}/runnable.cpp"
    "${HERE}/runnable.h"
    "${HERE}/state.h"
    )

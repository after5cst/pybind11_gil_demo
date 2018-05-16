# Set current directory for adding sources
set (HERE ${CMAKE_CURRENT_LIST_DIR})

target_sources("${PROJECT_NAME}"
    PRIVATE
    "${HERE}/control.h"
    "${HERE}/input.h"
    "${HERE}/output.h"
    "${HERE}/runnable.h"
    "${HERE}/state.h"
    )


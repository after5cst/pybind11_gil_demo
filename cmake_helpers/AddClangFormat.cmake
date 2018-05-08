cmake_minimum_required(VERSION 3.3)

string(REPLACE ":" ";" _PATH $ENV{PATH})
foreach(p ${_PATH})
        file(GLOB cand ${p}/clang-format*)
        if(cand)
                set(CLANG_FORMAT_EXECUTABLE ${cand})
                set(CLANG_FORMAT_FOUND ON)
                execute_process(COMMAND ${CLANG_FORMAT_EXECUTABLE} -version OUTPUT_VARIABLE clang_out )
                string(REGEX MATCH .*\(version[^\n]*\)\n version "${clang_out}")
                set(CLANG_FORMAT_VERSION ${CMAKE_MATCH_1})
                break()
        else()
                set(CLANG_FORMAT_FOUND OFF)
        endif()

endforeach()

cmake_policy(SET CMP0057 NEW)
option(
    ENABLE_CLANG_FORMATTER
   "Use clang-formatter to format source code?"
    ${CLANG_FORMAT_FOUND}
)
message(STATUS "option ENABLE_CLANG_FORMATTER=" ${ENABLE_CLANG_FORMATTER})

if("${ENABLE_CLANG_FORMATTER}" MATCHES "ON")
    LocateProgram(clang-format CLANG_FORMAT_PATH)
    if (NOT TARGET format)
        add_custom_target(format)
    endif (NOT TARGET format)
endif("${ENABLE_CLANG_FORMATTER}" MATCHES "ON")

function(AddClangFormat TARGET)
    if(${ENABLE_CLANG_FORMATTER})
        get_property(TARGET_SOURCES TARGET ${TARGET} PROPERTY SOURCES)
        set(COMMENT "running C++ code formatter: clang-format")
        set(COMMENT "${COMMENT}\nworking directory: ${CMAKE_CURRENT_SOURCE_DIR}")
        set(COMMENT "${COMMENT}\nprocessing files:\n${TARGET_SOURCES}")

        set(FORMAT_TARGET "format-${TARGET}")

        # Try to format only C/C++ sources and headers
        set(CLANG_FORMAT_SOURCES "")
        set(CLANG_EXTENSIONS ".c" ".cpp" ".cc" ".C" ".cxx" "c++"
                             ".h" ".hpp" ".hh" ".H" ".hxx" "h++")
        foreach(ITEM ${TARGET_SOURCES})
            get_filename_component( FILEEXT "${ITEM}" EXT)
            if ("${FILEEXT}" IN_LIST CLANG_EXTENSIONS)
                list(APPEND CLANG_FORMAT_SOURCES "${ITEM}")
            endif("${FILEEXT}" IN_LIST CLANG_EXTENSIONS)
        endforeach()

        add_custom_target(
            ${FORMAT_TARGET}
            COMMAND "${CLANG_FORMAT_PATH}" -style=file -i ${CLANG_FORMAT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
        add_dependencies(${TARGET} ${FORMAT_TARGET})
        add_dependencies(format ${FORMAT_TARGET})
    endif(${ENABLE_CLANG_FORMATTER})
endfunction(AddClangFormat)


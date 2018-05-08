option(
    ENABLE_ADD_PYTHON_UNIT_TESTS
    "Use ctest with python unit tests to build a make_test target?"
    ON
)
message(STATUS "option ENABLE_ADD_PYTHON_UNIT_TESTS=" ${ENABLE_ADD_PYTHON_UNIT_TESTS})

function(AddPythonUnitTests UNIT_TEST_LABEL SOURCE_DIR TARGET_DIR)
    find_package( PythonInterp 3 REQUIRED )
    if(${ENABLE_ADD_PYTHON_UNIT_TESTS})
        add_test(NAME "${UNIT_TEST_LABEL}_test" COMMAND "${PYTHON_EXECUTABLE}"
            -m unittest discover -v -f
            -s "${SOURCE_DIR}"
            WORKING_DIRECTORY "${TARGET_DIR}")
    endif(${ENABLE_ADD_PYTHON_UNIT_TESTS})
endfunction(AddPythonUnitTests)


#pragma once
// Include the pybind11 headers, suppressing warnings
// that we don't allow in our own code.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "pybind11/include/pybind11/pybind11.h"
#pragma GCC diagnostic pop

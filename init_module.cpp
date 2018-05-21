#include "count.h"
#include "really_async.h"
#include "worker/launch.h"
#include "worker/job.h"

#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>

/**
 * @brief block_for_one_second
 * Wait for one second.  This leaves the GIL lock from Python
 * locked, so it will block *EVERYTHING* else in Python from
 * running.
 */
void block_for_one_second()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/**
 * @brief sleep_for_one_second
 * Wait for one second with the GIL unlocked.  This will allow
 * other Python threads to run and do things while the sleep
 * is happening.
 */
void sleep_for_one_second()
{
    pybind11::gil_scoped_release guard_;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

PYBIND11_MODULE(gild, module)
{
    module.doc() = R"pbdoc(
Proof of concept demo for threading with pybind11 and the GIL.
-----------------------
.. currentmodule:: gild
.. autosummary::
   :toctree: _generate

    block_for_one_second
    sleep_for_one_second
    minimal_worker_example
)pbdoc";

    module.def("block_for_one_second", &block_for_one_second, R"pbdoc(
Demonstrate that if the GIL is not released, a C++ function
will block Python from doing anything else (including other
C++ methods) until the function is complete.

Parameters
----------
None

Returns
----------
None
)pbdoc");

    module.def("sleep_for_one_second", &sleep_for_one_second, R"pbdoc(
Demonstrate that if the GIL is released, other Python
threads can continue to run while the C++ code continues
to keep the calling Python thread 'busy'.

Parameters
----------
None

Returns
----------
None
)pbdoc");

    worker::bind_worker_input(module);
    worker::bind_worker_job(module);
    worker::bind_worker_launch(module);
    worker::bind_worker_state(module);

    count::input::bind(module);
    count::output::bind(module);
}

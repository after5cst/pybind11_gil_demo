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

#ifdef MOJO_JOJO

worker::control minimal_worker_example(bool report_success, int sleep_ms)
{
    worker::control temp;
    temp.future = really_async(
        [](std::shared_ptr<worker::state> state, bool report_success,
           int sleep_ms) {
            *state = worker::state::setup;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            *state = worker::state::working;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            *state = worker::state::teardown;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            *state = report_success ? worker::state::complete
                                    : worker::state::incomplete;
        },
        temp.state, report_success, sleep_ms);
    while (worker::state::not_started == *temp.state)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return std::move(temp);
}
#endif // MOJO_JOJO

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

#ifdef MOJO_JOJO
    module.def("minimal_worker_example", &minimal_worker_example, R"pbdoc(
Demonstrate that C++ can launch a worker thread and return
to Python, and that Python can then call and check on the
status of that worker.

Parameters
----------
report_success : if true, the worker will report it succeeded
                 at the task it was performing.  Otherwise, it
                 will return false.  Default: true.

sleep_ms       : Number of milliseconds to sleep between stages
                 of the worker (a total of three times).
                 Default: 100 ms.

Returns
----------
A Control() object, which can be queried for the status
of the worker.)pbdoc",
               pybind11::arg("report_success") = true,
               pybind11::arg("sleep_ms") = 100);

    module.def("launch", &launch, R"pbdoc(
Launch a worker object to perform work in a C++ thread.  The
type of worker object depends on the type of input object
provided.

Returns
----------
The related worker object.)pbdoc",
               pybind11::arg("input"));

    worker::control::bind(module);
    worker::input::bind(module);

    count::input::bind(module);
    count::output::bind(module);
    count::worker_class::bind(module, "CountWorker");

    pybind11::class_<count::count_input>(module, "CountInput")
        .def(pybind11::init<>())
        .def_readwrite("start", &count::count_input::start)
        .def_readwrite("end", &count::count_input::end)
        .def_readwrite("delay_ms", &count::count_input::delay_ms);

    typedef std::shared_ptr<count::count_output> output_ptr_t;
    pybind11::class_<count::count_output, output_ptr_t>(module, "CountOutput")
        .def(pybind11::init<>())
        .def_property_readonly(
            "last", [](output_ptr_t arg) { return arg ? int{arg->last} : -1; });

    typedef count::runnable worker_t;
    typedef std::shared_ptr<worker_t> worker_ptr_t;
    pybind11::class_<worker_t, worker_ptr_t>(module, "CountWorker")
        .def(pybind11::init<count::count_input>())
        .def("abort", &worker_t::abort)
        .def("start", &worker_t::start)
        .def_property_readonly("state", [](worker_ptr_t arg) {
            if (arg)
                switch (arg->get_state())
                {
                case worker::state::not_started:
                    return "not started";
                case worker::state::setup:
                    return "setup";
                case worker::state::working:
                    return "working";
                case worker::state::teardown:
                    return "teardown";
                case worker::state::complete:
                    return "complete";
                case worker::state::incomplete:
                    return "incomplete";
                }
            return "unknown";
        });
#endif
}

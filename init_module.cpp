#include "worker/control.h"

#include "really_async.h"

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
    // std::cout << "Guard released" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // std::cout << "Guard about to aquire" << std::endl;
}

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

PYBIND11_MODULE(gild, module)
{
    module.doc() = R"pbdoc(
          Proof of concept demo for threading with pybind11 and the GIL.
      )pbdoc";

    module.def("block_for_one_second", &block_for_one_second,
               "Block Python for one second");
    module.def("sleep_for_one_second", &sleep_for_one_second,
               "Sleep this thread for one second");
    module.def("minimal_worker_example", &minimal_worker_example,
               "One-second example worker",
               pybind11::arg("report_success") = true,
               pybind11::arg("sleep_ms") = 100);

    worker::control::bind(module);

#if 0 // MOJO_JOJO
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

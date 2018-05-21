// ------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2018 after5cst
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------
#include "job.h"

worker::job::~job() { abort(DEFAULT_ABORT_TIMEOUT); }

bool worker::job::abort(int timeout_in_seconds)
{
    control->keep_working.clear();
    wait_for_result(timeout_in_seconds);
    return finished();
}

worker::job::clock_t::duration worker::job::elapsed() const
{
    auto result = clock_t::duration::zero();
    const auto epoch = clock_t::time_point{};
    auto start = clock_t::time_point{control->start_working};
    if (epoch != start)
    {
        auto end = clock_t::time_point{control->end_working};
        if (end == epoch)
        {
            end = clock_t::now();
        }
        result = end - start;
    }
    return result;
}

bool worker::job::finished() const
{
    auto result = false;
    switch (get_state())
    {
    case state::not_started:
    case state::setup:
    case state::working:
    case state::teardown:
        break;
    case state::complete:
    case state::incomplete:
        future.wait();
        result = true;
        break;
    }
    return result;
}

bool worker::job::wait_for_result(int timeout_in_seconds) const
{
    auto result = false;
    switch (get_state())
    {
    case state::not_started:
        // The thread hasn't started.  This means the
        // object is freestanding and probably won't ever be
        // associated with anything.  Technically, it's done
        // but with a failure.
        break;
    case state::setup:
    case state::working:
    case state::teardown:
    case state::complete:
    case state::incomplete:
        if (0 > timeout_in_seconds)
        {
            // No timeout set, wait "forever"
            future.wait();
            result = (state::complete == get_state());
        }
        else
        {
            switch (future.wait_for(std::chrono::seconds(timeout_in_seconds)))
            {
            case std::future_status::ready:
                result = (state::complete == get_state());
                break;
            case std::future_status::timeout:
                // We exhausted our wait time.  As a result, the
                // caller will have to try again and/or check
                // finished() to see what happened.  In any case,
                // we did not complete successfully in the window
                // given.
                break;
            case std::future_status::deferred:
                // IMPOSSIBLE if really_async was used.
                assert(false);
                // Still, if assertions aren't enabled, we report
                // that the worker didn't successfully finish.
                break;
            }
            break;
        }
    }
    return result;
}

pybind11::module &worker::bind_worker_job(pybind11::module &module)
{
    pybind11::class_<job> obj(module, "Job", R"pbdoc(
        Job management object.  Returned from launch().

        When the object is deleted (reference count reaches 0), if
        the job is not finished it will be aborted, and the Python
        thread will block until the job is finished.
        )pbdoc");

    obj.def_property_readonly(
        "elapsed", &job::elapsed,
        "Time spent in the working state.  Updated in real time by Job");

    obj.def_property_readonly("finished", &job::finished,
                              "Set to True when job no longer running");

    obj.def_readonly("input", &job::input,
                     "Copy of job-specific input parameters");

    obj.def_readonly(
        "output", &job::output,
        "Job-specific output object.  Updated in real time by Job");

    obj.def("wait_for_result", &job::wait_for_result,
            pybind11::arg("timeout_in_seconds") = -1,
            "Wait until the job is completed with timeout value");

    obj.def_property_readonly(
        "state", &job::get_state,
        "State of the Job thread.  Uqpdated in real time by Job");

    return module;
}

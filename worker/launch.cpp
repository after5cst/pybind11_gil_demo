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
#include "launch.h"
#include "input.h"
#include "job.h"
#include "really_async.h"

namespace
{
    class set_timestamp_at_scope_exit
    {
    public:
        set_timestamp_at_scope_exit(worker::job::time_point_t &time)
            : m_time(time)
        {
        }
        ~set_timestamp_at_scope_exit() { m_time = worker::job::clock_t::now(); }

        // Delete copy constructor
        set_timestamp_at_scope_exit(const set_timestamp_at_scope_exit &rhs) =
            delete;

        // Delete move constructor
        set_timestamp_at_scope_exit(set_timestamp_at_scope_exit &&rhs) = delete;

        // Delete copy assignment
        set_timestamp_at_scope_exit
        operator=(const set_timestamp_at_scope_exit &rhs) = delete;

        // Delete move assignment
        set_timestamp_at_scope_exit
        operator=(set_timestamp_at_scope_exit &&rhs) = delete;

    private:
        worker::job::time_point_t &m_time;
    };

    void run_job(std::unique_ptr<worker::runnable> runnable,
                 worker::job::control_ptr_t control)
    {
        if (worker::state::not_started != control->state)
        {
            // This should be impossible.  Somebody's broken the source.
            assert(false);
            control->state = worker::state::incomplete;
            throw std::runtime_error("[DEVELOPER] New job != not_started");
        }

        auto success = false;
        try
        {
            control->state = worker::state::setup;
            success = runnable->on_setup();
            if (success)
            {
                control->state = worker::state::working;
                control->start_working = worker::job::clock_t::now();
                set_timestamp_at_scope_exit end_working(control->end_working);
                success = runnable->on_working(control->keep_working);
            }
        }
        catch (...)
        {
            // Want to teardown despite error, but ignore further errors.
            try
            {
                control->state = worker::state::teardown;
                runnable->on_teardown();
            }
            catch (...)
            {
                // IGNORE!
            }
            success = false;
            control->state = worker::state::incomplete;
            throw;
        }

        try
        {
            control->state = worker::state::teardown;
            success = runnable->on_teardown() && success;
        }
        catch (...)
        {
            success = false;
            control->state = worker::state::incomplete;
            throw;
        }

        control->state =
            success ? worker::state::complete : worker::state::incomplete;
    }
}

pybind11::object worker::launch(worker::input *input)
{
    auto job = std::make_unique<worker::job>();
    auto job_data = input->get_job_data();

    // Weirdly, atomic_flag is difficult to initialize within a
    // structure.  Specifically, it doesn't seem to be able to be
    // done in C++ by standard.  So we will set it here
    job->control->keep_working.test_and_set();

    job->input = std::move(job_data.python_input);
    job->output = std::move(job_data.python_output);
    job->future = really_async(run_job, std::move(job_data.runnable_object),
                               job->control);

    auto retries = 0;
    const auto MAX_RETRIES = 100;
    do
    {
        if (worker::state::not_started == job->get_state())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            // The worker has started.  We're done waiting.
            break;
        }
    } while (++retries < MAX_RETRIES);
    if (MAX_RETRIES == retries)
    {
        throw std::runtime_error("Launched thread did not start");
    }

    return pybind11::cast(job.release());
}

pybind11::module &worker::bind_worker_launch(pybind11::module &module)
{
    module.def("launch", &worker::launch, R"pbdoc(
Launch a Job object to perform work in a C++ thread.

Returns
----------
A Job object.

Note that if a Job object goes out of scope, the Job is
aborted and the Python thread waits for the Job to finish.
This means that the following code is (1) is effectively
synchronous and (2) immediately tries to abort the Job
it created:

launch(MyJob()) # THIS IS BAD!

As a result, the caller should assign the result of the launch
function to a variable, so the job can execute, e.g.:

# THIS IS BETTER!
my_job = launch(MyJob())
if not my_job.wait_for_result(60):
    raise RuntimeError("Job didn't complete successfully in 1 minute!!")
)pbdoc",
               pybind11::arg("input"));
    return module;
}

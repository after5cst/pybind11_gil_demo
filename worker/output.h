#ifndef WORKER_OUTPUT_H
#define WORKER_OUTPUT_H
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
#include "pybind11/chrono.h"
#include "./state.h"

namespace worker
{

    typedef std::atomic<std::chrono::time_point> thread_safe_time_point;

    ///
    /// @brief The base output struct
    ///
    /// The worker framework requires an output structure to be used.
    /// The worker will write output data to this structure for the
    /// calling Python environment to read either while running or
    /// after completion.
    ///
    /// This (derived) struct will be created as a shared pointer, and
    /// usable both by the launched runnable thread and by Python's
    /// own thread(s).
    ///
    /// A side effect of the "read while running" is that all variables
    /// in this struct and derivation must be thread-safe in some manner.
    ///
    struct output
    {
        ///
        /// \brief Return the time spent in runnable::run()
        ///
        /// An outside (Python) thread may want to measure how much
        /// time is being spent (or was spent) running the task.  This
        /// function calculates the value based on the state of the
        /// runnable and timestamps taken by the runnable.
        ///
        /// Note that time spent in setup() and teardown() is not
        /// included in this total.
        ///
        /// \return The elapsed time.
        ///
        std::chrono::duration elapsed() const
        {
            switch (runnable_state)
            {
            case state::not_started:
            case state::setup:
                break;
            case state::working:
                return std::chrono::steady_clock::now() - start_run;
            // no break needed, return called
            case state::teardown:
            case state::complete:
            case state::incomplete:
                return end_run - start_run;
                // no break needed, return called
            }
            return std::chrono::duration::zero();
        }

        /// @brief The start time for the related runnable::run().
        thread_safe_time_point start_run;
        /// @brief The end time for the related runnable::run().
        thread_safe_time_point end_run;

        std::atomic<state> runnable_state;

#if 0 // DOESN'T BELONG HERE -- NEED TO MOVE (TODO)
#include <future>
    /// @brief The end pass/fail from the related the runnable::run() function.
    ///
    /// See C++ documentation of std::async and std::future to understand
    /// the potential blocking scenarios if called.
    ///
    /// The problematic C++ behavior of blocking when the future goes
    /// out of scope here is avoided because the runnable will
    /// get a shared pointer to this structure, so the future cannot
    /// go out of scope until the runnable is complete.
    std::future<bool> future;
#endif
    };

} // end namespace worker

#endif // WORKER_OUTPUT_H

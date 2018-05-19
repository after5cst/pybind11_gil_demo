#ifndef WORKER_JOB_H
#define WORKER_JOB_H
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
#include "./state.h"

#include <future>

namespace worker
{
    struct job final
    {
        typedef std::chrono::steady_clock clock_t;
        typedef std::atomic<clock_t::time_point> time_point_t;
        struct control_t
        {
            std::atomic<worker::state> state = {worker::state::not_started};
            std::atomic_flag keep_working = {};

            /// @brief The start time for the related runnable::working().
            time_point_t start_working = {clock_t::time_point{}};
            /// @brief The end time for the related runnable::working().
            time_point_t end_working = {clock_t::time_point{}};
        };
        typedef std::shared_ptr<control_t> control_ptr_t;

        std::future<void> future = {};
        control_ptr_t control = {std::make_shared<control_t>()};
        pybind11::object input = {};
        pybind11::object output = {};

        enum
        {
            DEFAULT_ABORT_TIMEOUT = -1
        };

        /**
         * @brief Request the worker to abort and wait
         * @param timeout_in_seconds The timeout value in seconds.
         *        If this value is exceeded, the wait is abandoned.
         * @return Returns true if the worker is finished.
         */
        bool abort(int timeout_in_seconds = DEFAULT_ABORT_TIMEOUT);

        /**
         * @brief Return the time spent in runnable::run()
         *
         * An outside (Python) thread may want to measure how much
         * time is being spent (or was spent) running the Job.  This
         * function calculates the value based on the state of the
         * runnable and timestamps taken by the runnable.
         *
         * Note that time spent in setup() and teardown() is not
         * included in this total.
         *
         * @return The elapsed time
         */
        clock_t::duration elapsed() const;

        /**
         * @brief Return true if worker thread is finished and result is ready.
         * @return true if result is ready, false otherwise.
         */
        bool finished() const;

        /**
         * @brief get_state
         * @return The current state of the job thread
         */
        worker::state get_state() const { return control->state; }

        /**
         * @brief Wait for the worker to complete
         * @param timeout_in_seconds The timeout value in seconds.
         *        If this value is exceeded, the wait is abandoned
         *        and false is returned.
         * @return Return true if the worker is finished and the
         *        task was completed, false otherwise.
         */
        bool wait_for_result(int timeout_in_seconds) const;

        // We want to auto-abort at object destruction.
        // Because of the 'rule of 5', this now forces us to specify
        // all five additional forms.  In this case, the object is
        // lazy and just disallows any form of movement or copy.
        job() = default;
        job(const job &rhs) = delete;
        job(job &&rhs) = delete;
        job &operator=(const job &rhs) = delete;
        job &operator=(job &&rhs) = delete;
        ~job();
    };

    pybind11::module &bind_worker_job(pybind11::module &module);

} // end namespace worker

#endif // WORKER_JOB_H

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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "pybind11/chrono.h"
#pragma GCC diagnostic pop
#include "./state.h"

#include <atomic>
#include <chrono>
#include <mutex>

namespace worker
{
    typedef std::chrono::steady_clock clock_t;
    typedef std::atomic<clock_t::time_point> time_point_t;

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
        clock_t::duration elapsed() const
        {
            auto result = clock_t::duration::zero();
            const auto epoch = clock_t::time_point{};
            auto start = clock_t::time_point{start_run};
            if (epoch != start)
            {
                auto end = clock_t::time_point{end_run};
                if (end == epoch)
                {
                    end = clock_t::now();
                }
                result = end - start;
            }
            return result;
        }

        /// @brief The start time for the related runnable::run().
        time_point_t start_run = {clock_t::time_point{}};
        /// @brief The end time for the related runnable::run().
        time_point_t end_run = {clock_t::time_point{}};

        // pybind11 helpers to create Python wrapper object.
        typedef pybind11::class_<output> class_t;

#error REMEMBER TO DEAL WITH INHERITANCE IN INPUT/OUTPUT/CONTROL
        // OBJECTS.
        static pybind11::module &bind(pybind11::module &module)
        {
            class_t obj(module, "WorkerOutput");
            obj.def_property_readonly("elapsed", &output::elapsed);
            return module;
        }
    };

} // end namespace worker

#endif // WORKER_OUTPUT_H

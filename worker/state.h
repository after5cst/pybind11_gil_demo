#ifndef WORKER_STATE_H
#define WORKER_STATE_H
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
#include "include_pybind11.h"

namespace worker
{

    ///
    /// \brief Runnable states.
    ///
    /// The runnable will walk through a number of states during its lifetime.
    /// This enumeration shows which state the runnable is currently executing.
    ///
    /// not_started -> setup +--> working -> teardown +--> complete
    ///                      |                 ^      |
    ///                      |                 |      |
    ///                      +----on error-----+      +--> incomplete
    ///
    enum class state
    {
        not_started, ///< Thread execution has not begun
        setup,       ///< Setup for task is being performed
        working,     ///< Task currently in progress
        teardown,    ///< Teardown after task being performed
        complete,    ///< Task completed as requested
        incomplete   ///< Task did not complete because of error
    };

    inline pybind11::module &bind_worker_state(pybind11::module &module)
    {
        pybind11::enum_<state>(module, "State", pybind11::arithmetic(), R"pbdoc(
State enumeration for Job() objects.
)pbdoc")
            .value("NOT_STARTED", state::not_started,
                   "Job has not yet been started")
            .value("SETUP", state::setup,
                   "Performing Job-specific tasks prior to execution")
            .value("WORKING", state::working, "Performing Job tasks")
            .value("TEARDOWN", state::teardown,
                   "Performing Job-specific tasks after execution")
            .value("COMPLETE", state::complete,
                   "Job finished, all Job tasks complete")
            .value("INCOMPLETE", state::incomplete,
                   "Job finished, some Job tasks not complete");
        return module;
    }

} // end namespace worker

#endif // WORKER_STATE_H

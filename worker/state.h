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

    inline const char *as_string(state state_to_convert)
    {
        switch (state_to_convert)
        {
        case state::not_started:
            return "not started";
        case state::setup:
            return "setup";
        case state::working:
            return "working";
        case state::teardown:
            return "teardown";
        case state::complete:
            return "complete";
        case state::incomplete:
            return "incomplete";
        }
        return "unknown";
    }

} // end namespace worker

#endif // WORKER_STATE_H

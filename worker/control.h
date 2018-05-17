#ifndef WORKER_CONTROL_H
#define WORKER_CONTROL_H
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
#include "./state.h"

#include <assert.h>
#include <future>
#include <memory>

namespace worker
{
    struct control
    {
        std::future<void> future = std::future<void>();
        std::shared_ptr<worker::state> state =
            std::make_shared<worker::state>(worker::state::not_started);

        /**
         * @brief Return true if worker thread is finished and result is ready.
         * @return true if result is ready, false otherwise.
         */
        bool finished() const
        {
            if (nullptr == state)
            {
                assert(false);
                return false;
            }
            auto result = false;
            switch (*state)
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

        /**
         * @brief Wait for the worker to complete
         * @param timeout_in_seconds The timeout value in seconds.
         *        If this value is exceeded, the wait is abandoned
         *        and false is returned.
         * @return Return true if the worker is finished and the
         *        task was completed, false otherwise.
         */
        bool wait_for_result(int timeout_in_seconds) const
        {
            if (nullptr == state)
            {
                return false;
            }
            auto result = false;
            switch (*state)
            {
            case state::not_started:
                // The thread hasn't started.  This means the Control()
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
                    result = (state::complete == *state);
                }
                else
                {
                    switch (future.wait_for(
                        std::chrono::seconds(timeout_in_seconds)))
                    {
                    case std::future_status::ready:
                        result = (state::complete == *state);
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

        // pybind11 helpers to create Python wrapper object.
        typedef control container;
        typedef pybind11::class_<container> class_;

        static pybind11::module &bind(pybind11::module &module)
        {
            class_ obj(module, "Control");
            obj.def(pybind11::init<>());
            bind(obj);
            return module;
        }

        template <typename PARENT_CLASS> static void bind(PARENT_CLASS &obj)
        {
            obj.def_property_readonly("finished", &container::finished);
            obj.def("wait_for_result", &container::wait_for_result,
                    pybind11::arg("timeout_in_seconds") = -1);
            obj.def_property_readonly("state", [](const control &arg) {
                return worker::as_string(*arg.state);
            });
        }
    };

} // end namespace worker

#endif // WORKER_CONTROL_H

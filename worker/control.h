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
        std::future<void> future;
        std::shared_ptr<worker::state> state;

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

        bool join() const
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
                future.wait();
                result = (state::complete == *state);
                break;
            }
            return result;
        }

        // We want to automatically 'join' on destructor.  That requires
        // that (by rule of five) we define not only the destructor, but
        // all five (constructor, assignment constructor, assignment operator,
        // move constructor, move assignment operator)

        // Default constructor
        control()
            : future(),
              state(std::make_shared<worker::state>(state::not_started))
        {
        }
        control(const control &rhs) = delete;            // No copy constructor
        control &operator=(const control &rhs) = delete; // No copy assignment
        // Move constructor
        control(control &&rhs)
            : future(std::move(rhs.future)), state(std::move(rhs.state))
        {
        }
        control &operator=(control &&rhs)
        {
            if (this != &rhs)
            {
                future = std::move(rhs.future);
                state = std::move(rhs.state);
            }
            return *this;
        }

        ~control() { join(); }

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

        static class_ &bind(class_ &obj)
        {
            obj.def_property_readonly("finished", &container::finished);
            obj.def_property_readonly("result", &container::join);
            obj.def_property_readonly("state", [](const control &arg) {
                return worker::as_string(*arg.state);
            });
            return obj;
        }
    };

} // end namespace worker

#endif // WORKER_CONTROL_H

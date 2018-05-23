#ifndef WORKER_INPUT_H
#define WORKER_INPUT_H
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
#include "runnable.h"

namespace worker
{
    struct job_data
    {
        pybind11::object python_input = {};
        pybind11::object python_output = {};
        std::unique_ptr<runnable> runnable_object = {};
    };

    ///
    /// \brief The base input struct
    ///
    struct input
    {
        virtual ~input() = default;
        virtual job_data get_job_data() const = 0;
        virtual std::string get_str() const = 0;
        virtual std::string get_repr() const = 0;
    };

    inline pybind11::module &bind_worker_input(pybind11::module &module)
    {
        pybind11::class_<input> obj(module, "_JobInput");
        obj.def("__repr__", &input::get_repr);
        obj.def("__str__", &input::get_str);
        return module;
    }

} // end namespace worker

#endif // WORKER_INPUT_H

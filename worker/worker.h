#ifndef WORKER_WORKER_H
#define WORKER_WORKER_H
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
#include "./control.h"

namespace worker
{
    class runnable;

    template <typename INPUT_T, typename OUTPUT_T>
    struct worker : public control
    {
        typedef INPUT_T input_t;
        typedef std::shared_ptr<OUTPUT_T> output_ptr_t;
        typedef worker<INPUT_T, OUTPUT_T> worker_t;

        input_t input = {};
        output_ptr_t output = {};

        // pybind11 helpers to create Python wrapper object.
        typedef worker container;
        typedef pybind11::class_<container, std::shared_ptr<container>>
            shared_class;

        static pybind11::module &bind(pybind11::module &module,
                                      const char *object_name)
        {
            shared_class obj(module, object_name);
            obj.def(pybind11::init<>());
            control::bind(obj);
            obj.def_readonly("input", &container::input);
            obj.def_readonly("output", &container::output);
            return module;
        }
    };

} // end namespace worker

#endif // WORKER_WORKER_H

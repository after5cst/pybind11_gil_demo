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
#include "./output.h"
#include "enable_shared_from_this.h"
#include "really_async.h"

namespace worker
{
    typedef std::shared_ptr<std::atomic_bool> shared_bool;

    const char *notes = R"V0G0N(
        Basic idea here is that this object is created in the worker thread.
        As a result, it doesn't need a shared pointer or anything.  The
        constructor gets the data for the input and output.

        The 'run' method would then be invoked and work would be done.

        I'm not thinking this through clearly enough right now:
        The Python worker object will have the input and output type,
        but the vagueness of how to deal with the future and the quit
        conditions has to be thought through carefully.
)V0G0N";

    template <typename INPUT_T, typename OUTPUT_T> class runnable
    {
    protected:
        typedef INPUT_T input_t;
        typedef OUTPUT_T output_t;
        typedef std::shared_ptr<output_t> output_ptr_t;

        const input_t inputs;
        output_ptr_t outputs;

        runnable(input_t data_in, output_ptr_t data_out)
            : inputs(data_in), outputs(data_out)
        {
        }
        const runnable &operator=(const runnable &rhs) = delete;
        runnable(const runnable &rhs) = delete;
        virtual ~runnable() = default;

        virtual bool on_setup(const input_t &data_in, output_ptr_t data_out)
        {
            return true;
        }
        virtual bool on_working(const input_t &data_in, output_ptr_t data_out)
        {
            return true;
        }
        virtual bool on_teardown(const input_t &data_in, output_ptr_t data_out)
        {
            return true;
        }

    public:
        typedef void TODO_OUTPUT_T;
        TODO_OUTPUT_T run(shared_bool keep_running)
        {
            // Steal from run below.
        }
    };

    template <typename INPUT_T, typename OUTPUT_T>
    class runnable2
        : public enable_shared_from_this<runnable<INPUT_T, OUTPUT_T>>
    {
        typedef enable_shared_from_this<runnable<INPUT_T, OUTPUT_T>> baseclass;

    public:
        typedef std::shared_ptr<runnable> pointer_t;
        typedef INPUT_T input_t;
        typedef OUTPUT_T output_t;
        typedef std::shared_ptr<output_t> output_ptr_t;

        output_ptr_t start()
        {
            m_output = std::make_shared<output_t>();
            m_output->last = m_input.start;

            m_output->future = really_async(
                [](pointer_t shared_this) { return shared_this->run(); },
                baseclass::shared_from_this());

            return m_output;
        }

        worker::state get_state() const
        {
            return m_output ? worker::state{m_output->state}
                            : worker::state::not_started;
        }

        void abort() { m_keep_going = false; }

    protected:
        void set_state(worker::state state)
        {
            if (m_output)
            {
                m_output->state = state;
            }
            else
            {
                throw std::runtime_error(
                    "State cannot be set: no memory allocation");
            }
        }

        runnable(const input_t &input)
            : m_input(input), m_output(), m_keep_going(true)
        {
        }

        const input_t m_input;
        std::shared_ptr<output_t> m_output;
        std::atomic<bool> m_keep_going;

    private:
        worker::state run()
        {
            assert(nullptr != m_output);
            if (worker::state::not_started != get_state())
            {
                // I assume that a worker can be executed only once.
                // Otherwise, state and resource management gets more
                // complicated.
                return worker::state::incomplete;
            }

            if (m_keep_going)
            {
                set_state(worker::state::setup);
                if (!on_setup())
                {
                    m_keep_going = false;
                }
            }

            if (m_keep_going)
            {
                set_state(worker::state::working);
                if (!on_working())
                {
                    m_keep_going = false;
                }
            }

            // Regardless of whether execution is allowed, teardown must
            // be performed.  Otherwise we might have resource leaks.
            set_state(worker::state::teardown);
            if (!on_teardown())
            {
                m_keep_going = false;
            }

            set_state(m_keep_going ? worker::state::complete
                                   : worker::state::incomplete);
            return get_state();
        }
    };

} // end namespace worker

#endif // WORKER_INPUT_H

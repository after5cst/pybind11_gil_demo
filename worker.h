#ifndef WORKER_H
#define WORKER_H

#include "enable_shared_from_this.h"
#include "include_pybind11.h"

#include <assert.h>
#include <atomic>
#include <future>

namespace worker
{
    enum class state
    {
        not_started,
        setup,
        working,
        teardown,
        complete,
        incomplete
    };

    struct runnable_output
    {
        std::future<worker::state> future;
        std::atomic<worker::state> state;
        runnable_output() : future(), state(worker::state::not_started) {}
    };
    template <typename INPUT_T, typename OUTPUT_T>
    class runnable : public enable_shared_from_this<runnable<INPUT_T, OUTPUT_T>>
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
        runnable(const runnable &two) = delete;
        const runnable &operator=(const runnable &two) = delete;
        virtual ~runnable() = default;

        virtual bool on_setup() { return true; }
        virtual bool on_working() { return true; }
        virtual bool on_teardown() { return true; }

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
}

#endif // WORKER_H

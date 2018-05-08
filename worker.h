#include "enable_shared_from_this.h"

#include <assert.h>
#include <atomic>
#include <future>

namespace worker
{
    enum class state
    {
        not_started,
        setup,
        running,
        teardown,
        passed,
        failed
    };

    struct output_t
    {
        std::future<worker::state> future;
        std::atomic<worker::state> state;
        output_t() : future(), state(worker::state::not_started) {}
    };
    template <typename INPUT_T, typename OUTPUT_T>
    class base : public enable_shared_from_this<base<INPUT_T, OUTPUT_T>>
    {
        typedef enable_shared_from_this<base<INPUT_T, OUTPUT_T>> baseclass;

    public:
        typedef std::shared_ptr<base> pointer_t;
        typedef INPUT_T input_t;
        typedef OUTPUT_T output_t;
        typedef std::shared_ptr<output_t> output_ptr_t;

        output_ptr_t start()
        {
            m_output = std::make_shared<output_t>();
            m_output->last = m_input.start;

            m_output->future = really_async(
                [](pointer_t shared_this) { return shared_this->execute(); },
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

        worker::state execute()
        {
            assert(nullptr != m_output);
            if (worker::state::not_started != get_state())
            {
                // I assume that a worker can be executed only once.
                // Otherwise, state and resource management gets more
                // complicated.
                return worker::state::failed;
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
                set_state(worker::state::running);
                if (!on_running())
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

            set_state(m_keep_going ? worker::state::passed
                                   : worker::state::failed);
            return get_state();
        }

    public:
        base(const input_t &input)
            : m_input(input), m_output(), m_keep_going(true)
        {
        }
        base(const base &two) = delete;
        const base &operator=(const base &two) = delete;
        virtual ~base() = default;

    protected:
        virtual bool on_setup() { return true; }
        virtual bool on_running()
        {
            while (m_keep_going && m_output->last < m_input.end)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_input.delay_ms));
                ++m_output->last;
            }
            return true;
        }
        virtual bool on_teardown() { return true; }

    private:
        const input_t m_input;
        std::shared_ptr<output_t> m_output;
        std::atomic<bool> m_keep_going;
    };
}

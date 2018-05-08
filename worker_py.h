#ifndef WORKER_PY_H
#define WORKER_PY_H

#include <atomic>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

// Taken from Scott Meyer's "Effective C++" (2015) to ensure
// the std::async call *acutally* is async (as opposed to default
// behavior which is much more nebulous).
template <typename F, typename... Ts>
inline auto really_async(F &&f, Ts &&... params)
{
    return std::async(std::launch::async, std::forward<F>(f),
                      std::forward<Ts>(params)...);
}

#pragma GCC diagnostic push
// Effective C++ (correctly) warns when a non-virtual destructor is
// in the base class of one with a virtual destructor.  However, the
// enable_shared_from_this *explicitly* is built for exactly this
// behavior.  Disable the GCC warning for the duration of these classes.
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic pop

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

    // #error BAH... template_from_base WANTS TO BE COPIED BY PYBIND11 !!!

    class base
    {
    public:
        base() : m_allow_execution(true), m_state(state::not_started) {}
        virtual void execute()
        {
            assert(state::not_started == m_state);

            m_state = state::setup;
            if (!(m_allow_execution.test_and_set() && on_setup()))
            {
                m_allow_execution.clear();
            }

            m_state = state::running;
            if (!(m_allow_execution.test_and_set() && on_running()))
            {
                m_allow_execution.clear();
            }

            m_state = state::teardown;
            // Regardless of whether execution is allowed, teardown must
            // be performed.  Otherwise we might have resource leaks.
            on_teardown();

            m_state = m_allow_execution.test_and_set() ? state::passed
                                                       : state::failed;
        }
        virtual state get_state() const { return m_state; }

    protected:
        virtual bool on_setup() { return true; }
        virtual bool on_running() { return true; }
        virtual void on_teardown() {}
        std::atomic_flag m_allow_execution;
        std::atomic<state> m_state;

        virtual ~base() = default;
    };

    template <typename INPUT_T, typename OUTPUT_T>
    class template_from_base : public base
    {
    public:
        typedef OUTPUT_T output_t;
        typedef INPUT_T input_t;
        typedef std::shared_ptr<output_t> output_ptr_t;

        template_from_base(const input_t &input) : m_input(input), m_output() {}
        output_ptr_t get_output() const { return m_output; }

    protected:
        input_t m_input;
        output_ptr_t m_output;
    };

    template <typename INPUT_T, typename OUTPUT_T> struct container
    {
        typedef template_from_base<INPUT_T, OUTPUT_T> worker_t;
        std::shared_ptr<worker_t> worker;
        std::future<void> start_future;

        container(const INPUT_T &input) : worker(), start_future()
        {
            worker = std::make_shared<worker_t>(input);
        }

        const char *get_state() const
        {
            if (worker)
                switch (worker->get_state())
                {
                case worker::state::not_started:
                    return "not started";
                case worker::state::setup:
                    return "setup";
                case worker::state::running:
                    return "running";
                case worker::state::teardown:
                    return "teardown";
                case worker::state::passed:
                    return "passed";
                case worker::state::failed:
                    return "failed";
                }
            return "unknown";
        }

        typename worker_t::output_ptr_t start()
        {
            if (!worker)
            {
                throw std::runtime_error("No worker object");
            }
            if (state::not_started != worker->get_state())
            {
                throw std::runtime_error(
                    "Attempt to start worker a second time.");
            }
            start_future = really_async(
                [](std::shared_ptr<worker_t> object) {
                    return object->execute();
                },
                worker);
            return worker->get_output();
        }
    };

} // end namespace worker

struct counter_output_t
{
    std::atomic<int> counter;
};

struct counter_input_t
{
    int start;
    int end;
};

struct counter_worker
    : public worker::template_from_base<counter_input_t, counter_output_t>
{
    typedef template_from_base<counter_input_t, counter_output_t> baseclass;
    counter_worker(const input_t &input) : baseclass(input)
    {
        m_output = std::make_shared<counter_output_t>();
        m_output->counter = input.start;
    }

    bool on_running() override
    {
        auto counter = int{m_output->counter};
        while (m_allow_execution.test_and_set())
        {
            if (++counter > m_input.end)
            {
                break;
            }
            m_output->counter = counter;
        }
        return m_output->counter == m_input.end;
    }
};

#endif // WORKER_PY_H

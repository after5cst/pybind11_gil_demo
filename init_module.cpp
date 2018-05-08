#include "count.h"
#include "include_pybind11.h"

#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>

/**
 * @brief block_for_one_second
 * Wait for one second.  This leaves the GIL lock from Python
 * locked, so it will block *EVERYTHING* else in Python from
 * running.
 */
void block_for_one_second()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

/**
 * @brief sleep_for_one_second
 * Wait for one second with the GIL unlocked.  This will allow
 * other Python threads to run and do things while the sleep
 * is happening.
 */
void sleep_for_one_second()
{
    pybind11::gil_scoped_release guard_;
    // std::cout << "Guard released" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // std::cout << "Guard about to aquire" << std::endl;
}

#if 0
struct worker
{
    std::future<void> future = std::future<void>();
    void wait() { future.get(); }
};

void throw_up() { throw std::runtime_error("Throwing up!"); }
void wait_a_while(worker* worker)
{
    while(worker->futureObj.wait_for(std::chrono::milliseconds(0)) = std::future_status::timeout)
    {
        std::this_thread::sleep_for(std::chrono::seconds(11));
    }
    std::cout << "Done" << std::endl;
}
#elif 1

#include <atomic>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

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
// behavior.  Disable the GCC warning for the duration of this class.
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
template <typename T>
class enable_shared_from_this : public std::enable_shared_from_this<T>
{
public:
    virtual ~enable_shared_from_this() = default;
};
#pragma GCC diagnostic pop

class base_worker : public enable_shared_from_this<base_worker>
{
public:
    typedef std::shared_ptr<base_worker> pointer_t;
    typedef count::input_t input_t;
    typedef count::output_t output_t;
    typedef std::shared_ptr<output_t> output_ptr_t;

    output_ptr_t start()
    {
        m_output = std::make_shared<output_t>();
        m_output->last = m_input.start;

        m_output->future = really_async(
            [](pointer_t shared_this) { return shared_this->execute(); },
            shared_from_this());

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
            // Otherwise, state and resource management gets more complicated.
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

        set_state(m_keep_going ? worker::state::passed : worker::state::failed);
        return get_state();
    }

public:
    base_worker(const count::input_t &input)
        : m_input(input), m_output(), m_keep_going(true)
    {
    }
    base_worker(const base_worker &two) = delete;
    const base_worker &operator=(const base_worker &two) = delete;
    virtual ~base_worker() = default;

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
    const count::input_t m_input;
    std::shared_ptr<count::output_t> m_output;
    std::atomic<bool> m_keep_going;
};

#endif

PYBIND11_MODULE(gild, module)
{
    module.doc() = R"pbdoc(
          Proof of concept demo for threading with pybind11 and the GIL.
      )pbdoc";

    module.def("block_for_one_second", &block_for_one_second,
               "Block Python for one second");
    module.def("sleep_for_one_second", &sleep_for_one_second,
               "Sleep this thread for one second");

    pybind11::class_<count::input_t>(module, "CountInput")
        .def(pybind11::init<>())
        .def_readwrite("start", &count::input_t::start)
        .def_readwrite("end", &count::input_t::end)
        .def_readwrite("delay_ms", &count::input_t::delay_ms);

    typedef std::shared_ptr<count::output_t> output_ptr_t;
    pybind11::class_<count::output_t, output_ptr_t>(module, "CountOutput")
        .def(pybind11::init<>())
        .def_property_readonly(
            "last", [](output_ptr_t arg) { return arg ? int{arg->last} : -1; });

    typedef std::shared_ptr<base_worker> worker_ptr_t;
    pybind11::class_<base_worker, worker_ptr_t>(module, "CountWorker")
        .def(pybind11::init<count::input_t>())
        .def("abort", &base_worker::abort)
        .def("start", &base_worker::start)
        .def_property_readonly("state", [](worker_ptr_t arg) {
            if (arg)
                switch (arg->get_state())
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
        });

#ifdef JOJO
#error WHAT AM I DOING WRONG -- STD::ATOMIC DOES SEEM TO WORK!

    typedef worker::container<counter_worker, counter_input_t> counter_t;
    pybind11::class_<counter_t>(module, "Worker")
        .def(pybind11::init<const counter_input_t &>())
        .def_property_readonly("state", &counter_t::get_state)
        .def("start", &counter_t::start);

    pybind11::class_<std::atomic_flag>(module, "AtomicFlag")
        .def(pybind11::init<>())
        .def("clear",
             [](std::atomic_flag &a) {
                 a.clear();
                 return 0;
             })
        .def("test_and_set",
             [](std::atomic_flag &a) { return a.test_and_set(); });
#endif
}

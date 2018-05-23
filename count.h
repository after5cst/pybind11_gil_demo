#ifndef COUNT_H
#define COUNT_H

#include "worker/input.h"

#include <chrono>
#include <thread>

namespace count
{

    struct output
    {
        std::atomic<int> last = {0};
        static pybind11::module &bind(pybind11::module &module);
    };

    struct input : public worker::input
    {
        input(int start_, int end_, int delay_ms_)
            : start(start_), end(end_), delay_ms(delay_ms_)
        {
        }

        int start;
        int end;
        int delay_ms;

        worker::state fail_after = worker::state::incomplete;

        virtual worker::job_data get_job_data() const override;
        virtual std::string get_repr() const override;
        virtual std::string get_str() const override;
        static pybind11::module &bind(pybind11::module &module);
    };

    class runnable : public worker::runnable
    {
    public:
        runnable(input input_data, std::shared_ptr<output> output_data)
            : worker::runnable(), m_input(input_data), m_output(output_data)
        {
        }

        virtual bool on_setup() override;
        virtual bool on_working(std::atomic_flag &keep_working) override;
        virtual bool on_teardown() override;

    private:
        input m_input;
        std::shared_ptr<output> m_output;
    };
}

#endif // COUNT_H

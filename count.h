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
        int start = 0;
        int end = 100;
        int delay_ms = 1000;

        virtual worker::job_data get_job_data() const override;
        static pybind11::module &bind(pybind11::module &module);
    };

    class runnable : public worker::runnable
    {
    public:
        runnable(input input_data, std::shared_ptr<output> output_data)
            : worker::runnable(), m_input(input_data), m_output(output_data)
        {
        }

        virtual bool on_working(std::atomic_flag &keep_working) override;

    private:
        input m_input;
        std::shared_ptr<output> m_output;
    };
}

#endif // COUNT_H

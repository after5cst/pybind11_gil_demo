#ifndef COUNT_H
#define COUNT_H

#include "worker.h"

namespace count
{
    struct count_input
    {
        int start = 0;
        int end = 100;
        int delay_ms = 1000;

        static void init_pybind11(pybind11::module &module)
        {
            pybind11::class_<count::count_input>(module, "CountInput")
                .def(pybind11::init<>())
                .def_readwrite("start", &count::count_input::start)
                .def_readwrite("end", &count::count_input::end)
                .def_readwrite("delay_ms", &count::count_input::delay_ms);
        }
    };

    struct count_output : public worker::runnable_output
    {
        std::atomic<int> last;
        count_output() : worker::runnable_output(), last(0) {}

        static void init_pybind11(pybind11::module &module)
        {
            pybind11::class_<count::count_input>(module, "CountOutput")
                .def(pybind11::init<>())
                .def_readwrite("start", &count::count_input::start)
                .def_readwrite("end", &count::count_input::end)
                .def_readwrite("delay_ms", &count::count_input::delay_ms);
        }
    };

    class runnable : public worker::runnable<count_input, count_output>
    {
        typedef worker::runnable<count_input, count_output> baseclass;

    public:
        runnable(const input_t &input) : baseclass(input) {}

    protected:
        virtual bool on_working() override
        {
            while (m_keep_going && m_output->last < m_input.end)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_input.delay_ms));
                ++m_output->last;
            }
            return true;
        }
    };

} // end namespace count

#endif // COUNT_H

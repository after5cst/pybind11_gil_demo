#ifndef COUNT_H
#define COUNT_H

#include "worker.h"

namespace count
{
    struct input_t
    {
        int start = 0;
        int end = 100;
        int delay_ms = 1000;

        static void init_pybind11(pybind11::module &module)
        {
            pybind11::class_<count::input_t>(module, "CountInput")
                .def(pybind11::init<>())
                .def_readwrite("start", &count::input_t::start)
                .def_readwrite("end", &count::input_t::end)
                .def_readwrite("delay_ms", &count::input_t::delay_ms);
        }
    };

    struct output_t : public worker::output_t
    {
        std::atomic<int> last;
        output_t() : worker::output_t(), last(0) {}

        static void init_pybind11(pybind11::module &module)
        {
            pybind11::class_<count::input_t>(module, "CountInput")
                .def(pybind11::init<>())
                .def_readwrite("start", &count::input_t::start)
                .def_readwrite("end", &count::input_t::end)
                .def_readwrite("delay_ms", &count::input_t::delay_ms);
        }
    };

    class Worker : public worker::Base<input_t, output_t>
    {
        typedef worker::Base<input_t, output_t> baseclass;

    public:
        Worker(const input_t &input) : baseclass(input) {}

    protected:
        virtual bool on_running() override
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

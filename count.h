#ifndef COUNT_H
#define COUNT_H

#ifndef MOJO_JOJO
#include "worker/input.h"
#include "worker/output.h"
#include "worker/worker.h"

namespace count
{

    struct output : public worker::output
    {
        std::atomic<int> last = {0};

        // pybind11 helpers to create Python wrapper object.
        typedef output container;
        typedef pybind11::class_<container, std::shared_ptr<container>>
            shared_class;
        typedef worker::output baseclass;

        static pybind11::module &bind(pybind11::module &module)
        {
            shared_class obj(module, "CountOutput");
            obj.def(pybind11::init<>());
            obj.def_property_readonly("last", [](const container &arg) {
                return static_cast<int>(arg.last);
            });
            baseclass::bind(obj);
            return module;
        }
    };

    struct input;
    typedef worker::worker<input, output> worker_class;

    struct input : public worker::input
    {
        int start = 0;
        int end = 100;
        int delay_ms = 1000;

        virtual pybind11::object create_worker() override
        {
            return pybind11::cast(new worker_class{});
        }

        // pybind11 helpers to create Python wrapper object.
        typedef pybind11::class_<input, worker::input> class_t;

        static pybind11::module &bind(pybind11::module &module)
        {
            class_t obj(module, "CountInput");
            obj.def(pybind11::init<>());
            obj.def_readwrite("start", &input::start);
            obj.def_readwrite("end", &input::end);
            obj.def_readwrite("delay_ms", &input::delay_ms);
            return module;
        }
    };
}

#else // MOJO_JOJO
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
#endif // MOJO_JOJO

#endif // COUNT_H

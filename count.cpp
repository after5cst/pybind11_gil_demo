#include "count.h"

#include <chrono>
#include <thread>

pybind11::module &count::output::bind(pybind11::module &module)
{
    pybind11::class_<output, std::shared_ptr<output>> obj(module,
                                                          "CountOutput");
    obj.def(pybind11::init<>());
    obj.def_property_readonly(
        "last", [](const output &arg) { return static_cast<int>(arg.last); });
    return module;
}

pybind11::module &count::input::bind(pybind11::module &module)
{
    pybind11::class_<input, worker::input> obj(module, "Count");
    obj.def(pybind11::init<>());
    obj.def_readwrite("start", &input::start);
    obj.def_readwrite("end", &input::end);
    obj.def_readwrite("delay_ms", &input::delay_ms);
    return module;
}

worker::job_data count::input::get_job_data() const
{
    auto output_data = std::make_shared<output>();
    auto runnable_object =
        std::make_unique<count::runnable>(*this, output_data);

    worker::job_data result = {};
    result.python_input = pybind11::cast(*this);
    result.python_output = pybind11::cast(output_data);
    result.runnable_object = std::move(runnable_object);
    return std::move(result);
}

bool count::runnable::on_working(std::atomic_flag &keep_working)
{
    for (auto i = m_input.start; i <= m_input.end; ++i)
    {
        m_output->last = i;
        if (!keep_working.test_and_set())
        {
            // Told to abort what we were doing and stop!
            return false;
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(m_input.delay_ms));
    }
    return true;
}

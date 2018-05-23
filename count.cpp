#include "count.h"

#include <chrono>
#include <sstream>
#include <thread>

pybind11::module &count::output::bind(pybind11::module &module)
{
    pybind11::class_<output, std::shared_ptr<output>> obj(module,
                                                          "CountOutput");
    obj.def(pybind11::init<>());
    obj.def_property_readonly(
        "last", [](const output &arg) { return static_cast<int>(arg.last); },
        "The last number counted by the job thread");
    return module;
}

pybind11::module &count::input::bind(pybind11::module &module)
{
    pybind11::class_<input, worker::input> obj(module, "Count", R"pbdoc(
Asynchronous C++ job that counts between numbers with a delay.

This is a sample job, but does no useful work.  Not unless you think
incrementing a counter and sleeping are useful work.

This job does the following thing at each worker state:

SETUP:

  Delay for .delay_ms milliseconds to simulate setup tasks.

WORKING:

  Count from .start to .end (inclusive), delaying for .delay_ms
  between each step to simulate work.

TEARDOWN:

  Delay for .delay_ms milliseconds to simulate teardown tasks.

If .fail_after is set to State.SETUP, State.WORKING, or State.TEARDOWN,
then the job will fail at the end of the respective state.
        )pbdoc");
    obj.def(pybind11::init<int, int, int>(), pybind11::arg("start") = 1,
            pybind11::arg("end") = 100, pybind11::arg("delay_ms") = 1000);
    obj.def_readwrite("start", &input::start,
                      "The number to start counting from (inclusive)");
    obj.def_readwrite("end", &input::end,
                      "The final number in the counting sequence(inclusive)");
    obj.def_readwrite(
        "delay_ms", &input::delay_ms,
        "The sleep time in ms to be used in SETUP, WORKING, and TEARDOWN");
    obj.def_readwrite(
        "fail_after", &input::fail_after,
        "If set to SETUP, WORKING, or TEARDOWN, that state will fail.");
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

std::string count::input::get_repr() const
{
    return std::string("Count") + get_str();
}

std::string count::input::get_str() const
{
    std::stringstream sstr;
    sstr << "(start=" << start << ", end=" << end << ", delay_ms=" << delay_ms
         << ")";
    return std::move(sstr.str());
}

bool count::runnable::on_setup()
{
    // Simulate setup happening.
    std::this_thread::sleep_for(std::chrono::milliseconds(m_input.delay_ms));
    return m_input.fail_after != worker::state::setup;
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
    return m_input.fail_after != worker::state::working;
}

bool count::runnable::on_teardown()
{
    // Simulate teardown happening.
    std::this_thread::sleep_for(std::chrono::milliseconds(m_input.delay_ms));
    return m_input.fail_after != worker::state::teardown;
}

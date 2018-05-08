#ifndef COUNT_H
#define COUNT_H

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
}

namespace count
{
    struct input_t
    {
        int start = 0;
        int end = 100;
        int delay_ms = 1000;
    };

    struct output_t : public worker::output_t
    {
        std::atomic<int> last;
        output_t() : worker::output_t(), last(0) {}
    };

} // end namespace count

#endif // COUNT_H

#ifndef COUNT_H
#define COUNT_H

#include <atomic>
#include <future>

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

#ifndef WORKER_RUNNABLE_H
#define WORKER_RUNNABLE_H
// ------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2018 after5cst
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ------------------------------------------------------------------
#include "state.h"

#include <atomic>

namespace worker
{
    class runnable
    {
    protected:
        runnable() = default;

    public:
        /**
         * @brief This method is a 'hook' for derived classes.  When
         *        the worker starts it will call this method to
         *        perform any worker-specific setup.
         *
         *        If an exception is thrown from this method, it is
         *        handled as if false (setup failed) was returned
         *        from this method.
         *
         * @return True if setup was successful.  If setup was successful
         *        then on_working() will be called.  If not, on_working()
         *        will be skipped, on_teardown() will be called, and
         *        then the worker will stop.
         */
        virtual bool on_setup();

        /**
         * @brief This method is a 'hook' for derived classes.  When
         *        on_working() is called, the worker will perform the
         *        required task.
         *
         *        If an exception is thrown from this method, it is
         *        handled as if false (working failed) was returned
         *        from this method.
         *
         * @param keep_working  If cleared at any point, on_working
         *        should detect this value and exit as soon as is
         *        convenient.
         *
         * @return True if the task was completed successfully.
         *        Regardless of the outcome, on_teardown() will be
         *        called to clean up the worker state before exit.
         */
        virtual bool on_working(std::atomic_flag &keep_working) = 0;

        /**
         * @brief This method is a 'hook' for derived classes.  When
         *        on_teardown() is called, the worker should "undo"
         *        any setup performed in on_setup().
         *
         *        If an exception is thrown from this method, it is
         *        handled as if false (working failed) was returned
         *        from this method.
         *
         * @return True if teardown was completed successfully.
         *        Regardless of the outcome, the worker will finish
         *        shortly after this step.
         */
        virtual bool on_teardown();

        // because of the "rule of 5", and I need a virtual destructor
        // for derivation, I have to define all of the below.  By
        // default I assume the class is copyable and moveable although
        // in practice I doubt it will do either.
        runnable(const runnable &) = default;
        runnable(runnable &&) = default;
        runnable &operator=(const runnable &) = default;
        runnable &operator=(runnable &&) = default;
        virtual ~runnable() = default;
    };

} // end namespace worker

#endif // WORKER_RUNNABLE_H

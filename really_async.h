#ifndef REALLY_ASYNC_H
#define REALLY_ASYNC_H

#include <future>

// --------------------------------------------------------
// From "Effective Modern C++" by Scott Meyers (O'Reilly).
// Copyright 2015 Scott Meyers, 978-1-491-90399-5
//
// Item 38: Specify std::launch::async if asynchronicity is
//          essential.
// --------------------------------------------------------
template <typename F, typename... Ts>
inline auto really_async(F &&f, Ts &&... params)
{
    return std::async(std::launch::async, std::forward<F>(f),
                      std::forward<Ts>(params)...);
}

#endif // REALLY_ASYNC_H

//
// Created by borz7zy on 13.2.25..
//

#pragma once

#include <thread>
#include <atomic>

class my_thread {
public:
    template <typename Callable, typename... Args>
    explicit my_thread(Callable&& f, Args&&... args);

    ~my_thread();

    void join();
    void detach();
    bool is_done() const;

private:
    std::thread thread_;
    std::atomic<bool> done;
};

template <typename Callable, typename... Args>
my_thread::my_thread(Callable&& f, Args&&... args)
        : done(false) {
    thread_ = std::thread([this, f = std::forward<Callable>(f), args...]() {
        f(args...);
        done = true;
    });
}
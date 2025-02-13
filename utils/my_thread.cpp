//
// Created by borz7zy on 13.2.25..
//

#include "my_thread.h"
#include <iostream>
my_thread::~my_thread() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void my_thread::join() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void my_thread::detach() {
    if (thread_.joinable()) {
        std::cout << "Detaching thread..." << std::endl;
        thread_.detach();
    }
}

bool my_thread::is_done() const {
    return done;
}

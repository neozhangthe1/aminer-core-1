#pragma once
#include <functional>
#include <memory>
#include <thread>

// runnable should accept a int param, which indicates the index of that thread.
inline void dispatch_thread_group(std::function<void(int)> runnable, int thread_count) {
    std::unique_ptr<std::thread[]> threads(new std::thread[thread_count]);

    for (int i = 0; i < thread_count; i++)
        threads[i] = std::thread(runnable, i);

    for (int i = 0; i < thread_count; i++)
        threads[i].join();
}

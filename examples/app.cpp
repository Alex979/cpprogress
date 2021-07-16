#include <chrono>
#include <iostream>
#include <thread>

#include "progress.hpp"

using namespace std::chrono_literals;

Progress::Bar bar(Progress::BarOptions(500)
                      .name("Processing")
                      .format("{percent:.0f}% | {elapsed:%S}s"));

void increment_bar(int count, const std::chrono::milliseconds offset) {
    std::this_thread::sleep_for(offset);
    for (int i = 0; i < count; i++) {
        std::this_thread::sleep_for(10ms);
        bar.next();
    }
}

int main() {
    std::thread t1(increment_bar, 250, 0ms);
    std::thread t2(increment_bar, 250, 5ms);

    t1.detach();
    t2.detach();

    bar.run_until_full();

    std::cout << "Done\n";

    return 0;
}
#include <thread>

#include "progress.hpp"

void increment_bar(int count, Progress::Bar &bar) {
    for (int i = 0; i < count; i++) {
        bar.next();
    }
}

int main() {
    Progress::Bar bar(500, "Loading");

    std::thread t1(increment_bar, 250, std::ref(bar));
    std::thread t2(increment_bar, 250, std::ref(bar));

    bar.run_until_full();

    t1.join();
    t2.join();

    return 0;
}
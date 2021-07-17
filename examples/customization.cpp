#include <chrono>
#include <thread>

#include "progress.hpp"

using namespace std::literals::chrono_literals;

void fill_bar(Progress::Bar &bar, int count, std::chrono::nanoseconds delay) {
    bar.run_async();
    for (int i = 0; i < count; i++) {
        std::this_thread::sleep_for(delay);
        bar.next();
    }
    bar.done();
}

int main() {
    Progress::Bar bar(500);
    fill_bar(bar, 500, 300ns);

    Progress::Bar bar2(500, "Processing");
    fill_bar(bar2, 500, 300ns);

    Progress::Bar bar3(Progress::BarOptions(500)
                        .name("Please wait...")
                        .fill('\xfe')
                        .caps({"\xda[", "]\xbf"})
                        .format("{percent:.1f}%"));
    fill_bar(bar3, 500, 300ns);

    Progress::Bar bar4(Progress::BarOptions(500)
                        .name("Loooooong")
                        .fill('=')
                        .caps({"<<", ">>"})
                        .width(80)
                        .format("{current}"));
    fill_bar(bar4, 500, 300ns);

    Progress::Bar bar5(Progress::BarOptions(500)
                        .name("Funky!")
                        .fill('~')
                        .caps({"[", "]"})
                        .format("{elapsed:%S}s"));
    fill_bar(bar5, 500, 300ns);

    return 0;
}
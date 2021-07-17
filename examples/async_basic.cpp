#include "progress.hpp"

int main() {
    Progress::Bar bar(500, "Loading");

    bar.run_async();

    for (int i = 0; i < 500; i++) {
        bar.next();
    }

    bar.done();

    return 0;
}
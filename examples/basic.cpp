#include "progress.hpp"

int main() {
    Progress::Bar bar(300);
    for (int i = 0; i < 300; i++) {
        // Do something...

        // Increment the progress bar
        bar.next();
        // Display updated value every 10th iteration
        if (i % 10 == 0) bar.display();
    }
    // Notify completion
    bar.done();

    return 0;
}
#ifndef PROGRESS_H
#define PROGRESS_H

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#define FMT_HEADER_ONLY
#include "fmt/chrono.h"
#include "fmt/core.h"

namespace Progress {

namespace {
// Define private constants for BarOptions and Bar
const uint16_t DEFAULT_WIDTH{40};
const std::pair<std::string, std::string> DEFAULT_CAPS{"|", "|"};
const std::string DEFAULT_FORMAT{"{current}/{max}"};
const char DEFAULT_FILL{'#'};

}  // namespace

class Bar;

class BarOptions {
    friend class Bar;

   private:
    size_t max_;
    uint16_t width_;
    std::string name_;
    std::pair<std::string, std::string> caps_;
    std::string format_;
    char fill_;

   public:
    BarOptions(size_t max)
        : max_{max},
          width_{DEFAULT_WIDTH},
          caps_{DEFAULT_CAPS},
          format_{DEFAULT_FORMAT},
          fill_{DEFAULT_FILL} {};

    BarOptions &name(const std::string &name) {
        name_ = name;
        return *this;
    }

    BarOptions &width(uint16_t width) {
        width_ = width;
        return *this;
    }

    BarOptions &caps(const std::pair<std::string, std::string> &caps) {
        caps_ = caps;
        return *this;
    }

    BarOptions &format(const std::string &format) {
        format_ = format;
        return *this;
    }

    BarOptions &fill(char fill) {
        fill_ = fill;
        return *this;
    }
};

class Bar {
   private:
    size_t max;
    size_t current = 0;
    uint16_t width = 40;
    float percent = 0;
    bool displayed_once;
    bool is_done = false;
    size_t last_line_width = 0;
    std::string name;
    std::pair<std::string, std::string> caps;
    std::string format;
    char fill;
    std::mutex m;
    std::condition_variable cv;
    std::thread bar_thread;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::duration elapsed;

    void async_run_loop() {
        do {
            std::unique_lock<std::mutex> lk(m);
            display();
            size_t current_old = current;
            cv.wait(lk, [this, current_old] {
                return current > current_old || is_done;
            });
        } while (!is_done);
    }

   public:
    Bar(size_t max)
        : max{max},
          width{DEFAULT_WIDTH},
          caps{DEFAULT_CAPS},
          format{DEFAULT_FORMAT},
          fill{DEFAULT_FILL},
          start_time{std::chrono::steady_clock::now()} {};
    Bar(size_t max, const std::string &name)
        : max{max},
          width{DEFAULT_WIDTH},
          name{name},
          caps{DEFAULT_CAPS},
          format{DEFAULT_FORMAT},
          fill{DEFAULT_FILL},
          start_time{std::chrono::steady_clock::now()} {};
    Bar(const BarOptions &options)
        : max{options.max_},
          width{options.width_},
          name{options.name_},
          caps{options.caps_},
          format{options.format_},
          fill{options.fill_},
          start_time{std::chrono::steady_clock::now()} {};

    // Prints the progress bar to stdout, overwriting any output from a previous
    // call to display()
    void display() {
        // Clear the cursor
        std::cout << "\e[?25l";

        // If the bar has displayed once before, return the cursor to the start
        // of the line and overwrite
        if (displayed_once) {
            std::cout << "\r";
        }

        // Format name like so: "Name |#####...."
        if (!name.empty()) {
            std::cout << name << " ";
        }

        // Print the starting cap of the bar, the completed progress of the bar,
        // the remaining progress of the bar, and finally the end cap
        std::cout << caps.first;
        uint16_t current_progress = percent * width;
        for (uint16_t i = 0; i < current_progress; i++) {
            std::cout << fill;
        }
        for (uint16_t i = 0; i < width - current_progress; i++) {
            std::cout << " ";
        }
        std::cout << caps.second << " ";

        // Format the bar suffix info with the user-provided or default format
        // string
        std::string suffix = fmt::format(
            format, fmt::arg("current", current), fmt::arg("max", max),
            fmt::arg("percent", percent * 100), fmt::arg("elapsed", elapsed));
        std::cout << suffix;

        // Calculate the total width in characters of this display call. If it
        // isn't as long as the previous display, clear the remaining characters
        // with spaces
        size_t line_width = name.length() + caps.first.length() + width +
                            caps.second.length() + suffix.length() +
                            (name.length() == 0 ? 1 : 2);
        if (last_line_width > line_width) {
            for (size_t i = 0; i < last_line_width - line_width; i++) {
                std::cout << ' ';
            }
        }

        last_line_width = line_width;
        displayed_once = true;
        std::cout << std::flush;
    }

    // Increments the progress bar
    void next() {
        std::unique_lock<std::mutex> lk(m);
        current++;
        percent = (float)current / max;
        auto now = std::chrono::steady_clock::now();
        elapsed = now - start_time;
        lk.unlock();
        cv.notify_one();
    }

    // Print a newline and re-enable the cursor. Also joins the thread started
    // by run_async if applicable.
    void done() {
        std::unique_lock<std::mutex> lk(m);
        if (bar_thread.joinable()) {
            is_done = true;
            lk.unlock();
            cv.notify_one();
            bar_thread.join();
        }
        display();
        std::cout << "\n\e[?25h";
    }

    // Blocking function that continuously displays the updated state of the
    // progress bar until it is completely full
    void run_until_full() {
        do {
            std::unique_lock<std::mutex> lk(m);
            display();
            size_t current_old = current;
            cv.wait(lk, [this, current_old] { return current > current_old; });
        } while (current < max);

        done();
    }

    // Asynchronous function that continously displays the updated state of the
    // progress bar in a separate thread, until done() is called
    void run_async() {
        std::lock_guard<std::mutex> lk(m);
        if (bar_thread.joinable()) {
            std::cerr << "run_async() already called! Call done() to end the "
                         "progress bar."
                      << std::endl;
            return;
        }

        bar_thread = std::thread([this] { async_run_loop(); });
    }
};

}  // namespace Progress

#endif
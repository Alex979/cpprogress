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
#include "fmt/core.h"
#include "fmt/chrono.h"

namespace Progress {

class Bar;

class BarOptions {
    friend class Bar;

   private:
    int max_;
    int width_;
    std::string name_;
    std::pair<std::string, std::string> caps_;
    std::string format_;
    char fill_;

   public:
    BarOptions(int max)
        : max_(max),
          width_(40),
          caps_({"|", "|"}),
          format_("{current}/{max}"),
          fill_('#'){};

    BarOptions &name(const std::string &name) {
        name_ = name;
        return *this;
    }

    BarOptions &width(int width) {
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
    int max;
    int current = 0;
    int width = 40;
    float percent = 0;
    bool displayed_once;
    bool is_done = false;
    int last_line_width = 0;
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
            int current_old = current;
            cv.wait(lk, [this, current_old] {
                return current > current_old || is_done;
            });
        } while (!is_done);
    }

   public:
    Bar(int max, const std::string &name = "")
        : max(max),
          width(40),
          name(name),
          caps({"|", "|"}),
          format("{current}/{max}"),
          fill('#'),
          start_time(std::chrono::steady_clock::now()){};
    Bar(const BarOptions &options)
        : max(options.max_),
          width(options.width_),
          name(options.name_),
          caps(options.caps_),
          format(options.format_),
          fill(options.fill_),
          start_time(std::chrono::steady_clock::now()){};

    void display() {
        int i;
        int current_progress = percent * width;
        std::string suffix = fmt::format(format, fmt::arg("current", current),
                                         fmt::arg("max", max),
                                         fmt::arg("percent", percent * 100),
                                         fmt::arg("elapsed", elapsed));

        // Clear the cursor
        std::cout << "\e[?25l";

        if (displayed_once) {
            std::cout << "\r";
        }

        if (!name.empty()) {
            std::cout << name << " ";
        }

        std::cout << caps.first;
        for (i = 0; i < current_progress; i++) {
            std::cout << fill;
        }
        for (i = 0; i < width - current_progress; i++) {
            std::cout << " ";
        }
        std::cout << caps.second << " ";
        std::cout << suffix;

        int line_width = name.length() + caps.first.length() + width +
                         caps.second.length() + suffix.length() + name.length() == 0 ? 1 : 2;
        // If this line was not as long as the previous line, fill in spaces to
        // fully clear the previous output
        if (last_line_width > line_width) {
            for (i = 0; i < last_line_width - line_width; i++) {
                std::cout << ' ';
            }
        }
        last_line_width = line_width;
        displayed_once = true;
        std::cout << std::flush;
    }

    void next() {
        std::unique_lock<std::mutex> lk(m);
        current++;
        percent = (float)current / max;
        auto now = std::chrono::steady_clock::now();
        elapsed = now - start_time;
        lk.unlock();
        cv.notify_one();
    }

    // Print a newline and re-enable the cursor
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

    void run_until_full() {
        do {
            std::unique_lock<std::mutex> lk(m);
            display();
            int current_old = current;
            cv.wait(lk, [this, current_old] { return current > current_old; });
        } while (current < max);

        done();
    }

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
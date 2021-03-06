# cpprogress
This library is a work in progress and is NOT production ready. Any contributions are greatly appreciated.

**cpprogress** is a header-only C++ library for implementing progress bars in command-line applications. The progress bars are designed to be thread-safe, and highly customizable in appearance.

![Windows PowerShell 2021-07-16 23-24-33](https://user-images.githubusercontent.com/11466316/126024663-96998b8f-47ec-46bf-928f-19dccad3dd04.gif)

## Table of Contents
- [Table of Contents](#table-of-contents)
- [Usage](#usage)
- [Examples](#examples)
  - [Asynchronous execution](#asynchronous-execution)
- [Customization](#customization)
- [Formatting](#formatting)

## Usage
To use **cpprogress**, simply move the contents of the provided [include/](include/) directory into your own include directory and include the header like so:
```C++
#include "progress.hpp"
```

## Examples
The currently implemented progress bar requires a maximum value to be provided. Use `next()` to increment the progress bar, `display()` to display the current progress, and `done()` when finished.
```C++
#include "progress.hpp"

int main() {
    Progress::Bar bar(300);
    for (int i = 0; i < 300; i++) {
        // Do something...

        // Increment the progress bar
        bar.next();
        // Display updated progress
        bar.display();
    }
    // Notify completion
    bar.done();

    return 0;
}

// Output:
// |########################################| 300/300
```
The `display()` method is an expensive operation, so for the example above, it may be beneficial to only display every nth iteration if you have a large number of values, like so:
```C++
if (i % 10 == 0) bar.display();
```

### Asynchronous execution
The progress bar can display asynchronously, which keeps the `display()` method from slowing down the current process, and is also useful for multi-threaded applications. Simply call `run_async()`, iterate the progress bar with `next()`, and call `done()` when finished. The `display()` method is automatically called when using `run_async()`.

#### Basic Asynchronous Example
```C++
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

// Output:
// Loading |########################################| 500/500
```

#### Threaded Asynchronous Example
```C++
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

    bar.run_async();

    t1.join();
    t2.join();

    bar.done();

    return 0;
}

// Output:
// Loading |########################################| 500/500
```

#### Synchronous blocking example
For multi-threaded applications, there is also a blocking function that runs and displays the progress bar until it is full. Simply call `run_until_full()` and iterate the progress bar in your threads with `next()`. The progress bar will automatically call `done()` when it is full.

```C++
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
```

## Customization
The default constructor for `Progress::Bar` requires a maximum value of iterations, and optionally a name for the progress bar. For example, a progress bar defined like so:
```C++
Progress::Bar bar(300, "Processing");
```
would appear like this:
```
Processing |####################                    | 150/300
```

For futher customization, initialize the progress bar with a `BarOptions` instance like so:
```C++
Progress::Bar bar(Progress::BarOptions(300)
                        .name("Loading")
                        .fill('-')
                        .caps({"<<", ">>"})
                        .format("{percent:.1f}%"));
```
which will appear like this:
```
Loading <<--------------------                    >> 50.0%
```

Below is a list of all options that can be customized with a `BarOptions` instance.

|Option|Description|Example|
|------|-----------|-------|
|name  |Provides a name that will be displayed adjacent to the progress bar.|`.name("Loading")`|
|width |Sets the width of the progress bar.|`.width(60)`|
|fill  |Sets the character used for filling the progress bar.|`.fill('=')`|
|caps  |Sets the strings to be used for the start and end caps of the progress bar.|`.caps({"[", "]"})`|
|format|Defines the format used to display the numerical progress at the end of the progress bar.|See [Formatting](#formatting)

## Formatting
To use the `.format("...")` option when customizing a progress bar, provide a string that defines the format of the numerical progress while displaying. This uses the [{fmt}](https://fmt.dev/latest/index.html) library under the hood, and the syntax should be identical. 

For example, calling `.format("{count}")` will only display the count like so:
```
Processing |####################                    | 150
```
Calling `.format("{percent:.1f}%")` will display a percentage with one decimal place:
```
Processing |####################                    | 50.0%
```
Calling `.format("{current}/{max} {elapsed:%S}s")` will display the count and elapsed time in seconds:
```
Processing |#################################       | 416/500 13.043s
```

The available parameters accessible in format strings are:

|Format option|Description|
|-------------|-----------|
|current      |The current progress of the bar as a count|
|max          |The maximum count of the bar|
|percent      |The current progress of the bar as a percentage|
|elapsed      |The current elapsed time the bar has been running|

For further documentation on format strings, see the [{fmt} docs.](https://fmt.dev/latest/syntax.html)

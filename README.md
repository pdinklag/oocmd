# oocmd &ndash; Object-Oriented Command Line Parser for C++

This library provides an object-oriented command line parser for the entry point of your C++20 application.

It solves two problems that occur on a regular basis:

1. Parse the command line arguments given to the application
2. Store the parsed information directly into a (potentially hierarchical) structure defined by the application

While there are many libraries that allow parsing the command line, this one focuses on the structured storage. It allows defining the available command-line options in an intuitive, object-oriented way. Let's look at an example:

```cpp
#include <oocmd.hpp>

using namespace oocmd;

struct Addition : public ConfigObject {
   	int a = 0;
    int b = 0;
    
    Addition() : ConfigObject("Addition", "Adds two numbers and prints the result") {
        param('a', "first",  a, "The first summand.");
        param('b', "second", b, "The second summand.");
    }

    int run(Application const& app) {
        int sum = a + b;
        std::cout << sum << std::endl;
        return 0;
    }
};

int main(int argc, char** argv) {
    Addition add;
    return Application::run(add, argc, argv);
}
```

## License

tbd

## Usage

The library is header only, so all you need to do is make sure it's in your include path and:

```cpp
#include <oocmd.hpp>
```

In case you use CMake, you can embed this repository into yours (e.g., as a git submodule) and add it like so:

```cmake
add_subdirectory(path/to/oocmd)
```

You can then link against the `oocmd` interface library, which will automatically add the include directory to your target.

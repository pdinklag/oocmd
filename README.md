# oocmd &ndash; Object-Oriented Command Line Parser for C++

This library provides an object-oriented command- ine parser as the entry point of your C++20 application.

:construction:

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

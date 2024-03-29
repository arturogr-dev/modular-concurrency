# The Modular-Concurrency Project

___MODCNCY is a C++11-based collection of modular and reusable technologies to power multithreaded applications in multicore shared-memory architectures.___

## tl;dr

Is there room for this concurrency library? I believe so.

It is well-known that in the worlds of multithreading, synchronization, concurrency, parallelism, etc.:

- There is no such thing as _one-size-fits-all_ solution.
- There is sometimes a big cost associated with our pursuit of "performance", like attempting to create non-blocking algorithms. It is extremely difficult (and dangerous) to write concurrent code using only atomic operations. See [this](https://abseil.io/docs/cpp/atomic_danger).
- And still, there is no straightforward way to experiment with these multiple solutions. Sure, you can modify code (choosing between different locking synchronization primitives, for example. Maybe you want to add some fairness, try different policies?) and compile it every time you want to try out a different solution, which can greatly affect productivity. Or you can use a bunch of flags and/or macros and/or template specializations and make it harder to read and/or maintain it (and this is assuming we know what to expose at compile-time).

_The Modular-Concurrency Project_ is an open source effort that aims to provide a portable infrastructure to test and benchmark multiple concurrent applications in a very simple way with the help of a set of modular and reusable tools. True, some of these technologies are __experimental__, some of them might be just __folklore__. However, you will also find __practical__ and __production-level__ solutions to power your most demanding multithreaded applications.

This project does not intend to compete with other concurrency utility libraries like [abseil](https://github.com/abseil/abseil-cpp/tree/master/absl/synchronization) or [folly](https://github.com/facebook/folly/tree/main/folly/concurrency), but rather try to help improve them since __modcncy__ development philosophy is driven by testing, benchmarking and experimentation experience.

Let's build a __state-of-the-art__ core software technologies hub for multicore computing.

-- Arturo

__NOTE:__ This is an experimental project and it should be treated as such.

## Dependencies

- [Google Benchmark](https://github.com/google/benchmark)
- [Google Test](https://github.com/google/googletest)

## Getting Started

This describes a general process to install the `modcncy` library and its _submodule_ dependencies. For more specific instructions on each of the submodules, please visit their respective documentation.

### Clone the repository and its submodules

```bash
$ git clone --recurse-submodules https://github.com/arturogr-dev/modular-concurrency.git
```

### Build the `Google Test` framework dependency

```bash
$ cd modular-concurrency/third_party/googletest
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Build the `Google Benchmark` library dependency

```bash
$ cd modular-concurrency/third_party/benchmark
$ cmake -E make_directory "build"
$ cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
$ cmake --build "build" --config Release
```

If you want to check the build, you can run its tests:

```bash
$ cmake -E chdir "build" ctest --build-config Release
```

### Build the `modcncy` library

```bash
$ cd modular-concurrency/modcncy
$ make
```

This builds the library in

```
/modular-concurrency
  /modcncy
    /build
      /lib
        /libmodcncy.a
        ...
```

If you want to check the build, you can run its tests:

```bash
$ cd modular-concurrency/modcncy/tests
$ make
```

and its benchmarks:

```bash
$ cd modular-concurrency/modcncy/benchmarks
$ make
```

## Tools

- [Cpplint](https://github.com/cpplint/cpplint)

TODO(arturogr-dev): Finish README.md file.

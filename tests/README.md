# Panda3D test suite

This directory contains two test suites that share one directory layout:

- **Python tests** (`<package>/test_*.py`), run with pytest.  These test the
  behavior of the engine through its published (interrogate-generated) API.
- **C++ tests** (`<package>/test_*.cxx`), built into a single `run_cxx_tests`
  binary using the vendored [Catch2](https://github.com/catchorg/Catch2)
  framework in `catch2/`.  These test internal C++ machinery that is not
  reachable through the published API.

Both suites are discovered automatically; adding a new `test_*.py` or
`test_*.cxx` file under a package directory is all that is needed.

## Running the tests

To run the Python test suite, run:

    python -m pytest tests

(In sanitizer-instrumented or static builds, use the `run_pytest` binary in
the built directory instead, which embeds the interpreter.)

The Python test suite automatically collects the C++ tests as well.  If you
wish to run the C++ test suite separately, you can pass `--no-cxx-tests` to
pytest and run the C++ test suite separately as follows:

    built/bin/run_cxx_tests                        # run everything
    built/bin/run_cxx_tests "[express]"            # run one package's tests
    built/bin/run_cxx_tests --list-tests           # see what's there

In CMake builds, `ctest` is an alternative means to run both the Python and
C++ tests; with CMake 3.19+, it registers each C++ test case individually,
but the whole Python suite (excluding the C++ tests) is run as a single
`pytest` test.

## Writing tests

If the behavior under test is observable through the published API, new tests
**must be written in Python**.  The Python suite tests the contract that users
actually program against, runs against every wheel configuration on CI, and is
readable and runnable by the largest number of contributors.

A test may be written in C++ only if at least one of the following holds:

1. **The code is not published, and publishing it would serve no purpose
   other than testing.**  Do not add interrogate bindings that significantly
   bloat the exposed interface just to make something testable.
2. **The semantics under test do not exist at the Python level.**  Overload
   resolution, move semantics, template instantiation, implicit conversions,
   object lifetime and reference-count transfer, memory layout, compile-time
   behavior.  A binding cannot faithfully exercise these even when the class
   itself is published.
3. **The test needs control that the bindings cannot provide.**  Exact
   interleaving of pipeline stages or threads, stack vs. heap construction,
   injecting failures into internals mid-operation.

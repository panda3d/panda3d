# The C++ test suite, using the vendored Catch2 framework in catch2/.
# Tests are discovered automatically: any <package>/test_*.cxx file is
# compiled into the run_cxx_tests binary.
file(GLOB cxx_test_sources CONFIGURE_DEPENDS "*/test_*.cxx")
list(FILTER cxx_test_sources EXCLUDE REGEX "/catch2/")

add_executable(run_cxx_tests catch2/catch_amalgamated.cpp ${cxx_test_sources})
target_include_directories(run_cxx_tests PRIVATE catch2)
target_link_libraries(run_cxx_tests panda)

# Where supported, report each Catch2 test case as its own ctest test, with
# its tags as labels (so e.g. `ctest -L express` works).  catch_discover_tests
# requires CMake 3.19, and discovery runs the binary at build time, which is
# not possible when cross-compiling.
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.19" AND NOT CMAKE_CROSSCOMPILING)
  include(catch2/Catch.cmake)
  catch_discover_tests(run_cxx_tests ADD_TAGS_AS_LABELS)
else()
  add_test(NAME run_cxx_tests COMMAND run_cxx_tests)
endif()

if(NOT INTERROGATE_PYTHON_INTERFACE)
  return()
endif()

add_executable(run_pytest main.c)
target_link_libraries(run_pytest panda)
target_link_libraries(run_pytest Python::Python)

# Note, we do NOT test for pytest before adding this test.  If the user doesn't
# have pytest, we'd like for the tests to fail.

# In the Coverage configuration, we also require pytest-cov

# To prevent an empty argument from being passed in in non-Coverage builds,
# which will sometimes cause pytest to read all tests in the current directory,
# substitute a nonsensical argument that will be ignored instead
# The run_cxx_tests test above already runs the C++ suite directly, so don't
# have pytest run it again.
add_test(NAME pytest COMMAND run_pytest tests --no-cxx-tests $<IF:$<CONFIG:Coverage>,--cov=.,--ignore=nonexistent>
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

set_tests_properties(pytest PROPERTIES ENVIRONMENT "PYTHONPATH=${PANDA_OUTPUT_DIR}")

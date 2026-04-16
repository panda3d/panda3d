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
add_test(NAME pytest COMMAND run_pytest tests $<IF:$<CONFIG:Coverage>,--cov=.,--ignore=nonexistent>
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

set_tests_properties(pytest PROPERTIES ENVIRONMENT "PYTHONPATH=${PANDA_OUTPUT_DIR}")

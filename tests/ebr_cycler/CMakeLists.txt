add_executable(ebr_cycler ebr_cycler.cxx)
target_link_libraries(ebr_cycler p3framework)

find_package(Threads REQUIRED)
target_link_libraries(ebr_cycler Threads::Threads)

add_test(NAME ebr_cycler
  COMMAND ebr_cycler 5
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
set_tests_properties(ebr_cycler PROPERTIES
  LABELS "ebr_cycler"
  TIMEOUT 30
  ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib")

# Read-your-writes through the deferred-publish COW path, and prompt reclamation
# at quiescence -- the two correctness properties behind the Windows loadModel
# crash and the node-subclass GC test.  Forces pipeline-always-cow internally.
add_test(NAME ebr_cycler_read_your_writes
  COMMAND ebr_cycler --read-your-writes
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
add_test(NAME ebr_cycler_gc_lifetime
  COMMAND ebr_cycler --gc-lifetime
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
set_tests_properties(ebr_cycler_read_your_writes ebr_cycler_gc_lifetime PROPERTIES
  LABELS "ebr_cycler"
  TIMEOUT 30
  ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib")

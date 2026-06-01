add_executable(scenegraph_stress scenegraph_stress.cxx)
target_link_libraries(scenegraph_stress p3framework)

if(NOT BUILD_SHARED_LIBS)
  if(HAVE_GL)
    target_link_libraries(scenegraph_stress pandagl)
  endif()
  if(HAVE_TINYDISPLAY)
    target_link_libraries(scenegraph_stress p3tinydisplay)
  endif()
endif()

find_package(Threads REQUIRED)
target_link_libraries(scenegraph_stress Threads::Threads)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(_stress_runner "${CMAKE_CURRENT_SOURCE_DIR}/run.py")

# Each ctest case invokes run.py to run the binary `runs` times and
# summarize.  The binary builds its own geometry, so no model file,
# model-path, or loader-plugin setup is needed.
function(add_scenegraph_stress_test name runs)
  add_test(NAME scenegraph_stress.${name}
    COMMAND ${Python3_EXECUTABLE} ${_stress_runner}
            --binary $<TARGET_FILE:scenegraph_stress>
            --runs ${runs}
            -- ${ARGN}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  set_tests_properties(scenegraph_stress.${name} PROPERTIES
    LABELS "scenegraph_stress"
    TIMEOUT 60
    SKIP_RETURN_CODE 77
    ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib")
endfunction()

# Ops known to surface latent panda3d races (50-trial bisections):
#   s = stash/unstash    MutexPosixImpl::unlock owner assert (~3-27%)
#   r = reparent         same assertion family (~8%)
#   b = get_bounds       worker-thread reads × render pipeline (~13-100%)
# Each char is one slot; duplicates raise that op's weight.  The mix
# below holds the structural-mutation ratio (a/R) close to the original
# repro while still exercising every op.
set(STABLE_OPS "aahhhhpsrbR")

add_scenegraph_stress_test(flat 3
  --ops ${STABLE_OPS})
add_scenegraph_stress_test(deep 3
  --depth 5 --ops ${STABLE_OPS})
add_scenegraph_stress_test(deeper 3
  --depth 15 --ops ${STABLE_OPS})
add_scenegraph_stress_test(per_worker 3
  --depth 5 --per-worker --ops ${STABLE_OPS})
add_scenegraph_stress_test(heavy 2
  --depth 5 --workers 8 --iterations 1500 --timeout 20
  --ops ${STABLE_OPS})
add_scenegraph_stress_test(branchy 3
  --depth 5 --branching 4 --ops ${STABLE_OPS})
add_scenegraph_stress_test(per_worker_wide 3
  --depth 5 --per-worker --workers 8 --ops ${STABLE_OPS})
add_scenegraph_stress_test(extreme 2
  --depth 8 --branching 3 --workers 8 --iterations 1500
  --threading-model cull/draw --timeout 25 --ops ${STABLE_OPS})
add_scenegraph_stress_test(threaded_cull_draw 3
  --depth 5 --threading-model cull/draw --timeout 15
  --ops ${STABLE_OPS})
add_scenegraph_stress_test(threaded_cull 3
  --depth 5 --threading-model Cull --timeout 15
  --ops ${STABLE_OPS})

# Discovery cases: focused reproductions for whoever investigates each
# bug.  Labelled `scenegraph_known_flaky` so `ctest -L scenegraph_stress`
# (regex match) does NOT pick them up.  Run with
# `ctest -L scenegraph_known_flaky` or by name.
function(add_known_flaky_test name runs)
  add_test(NAME scenegraph_stress.${name}
    COMMAND ${Python3_EXECUTABLE} ${_stress_runner}
            --binary $<TARGET_FILE:scenegraph_stress>
            --runs ${runs}
            -- ${ARGN}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  set_tests_properties(scenegraph_stress.${name} PROPERTIES
    LABELS "scenegraph_known_flaky"
    TIMEOUT 180
    SKIP_RETURN_CODE 77
    ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib")
endfunction()

add_known_flaky_test(stash_focus 5
  --depth 5 --workers 8 --iterations 1500 --timeout 20 --ops ahpsR)
add_known_flaky_test(reparent_focus 5
  --depth 5 --per-worker --workers 8 --iterations 1500 --timeout 20
  --ops ahprR)
add_known_flaky_test(read_focus 5
  --depth 5 --threading-model Cull --timeout 20 --ops ahpbR)
add_known_flaky_test(full_mix 5
  --depth 5 --workers 8 --iterations 1500 --timeout 20 --ops ahpsrbR)

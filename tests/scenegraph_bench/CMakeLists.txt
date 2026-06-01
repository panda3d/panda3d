add_executable(scenegraph_bench scenegraph_bench.cxx)
target_link_libraries(scenegraph_bench p3framework)

if(NOT BUILD_SHARED_LIBS)
  if(HAVE_GL)
    target_link_libraries(scenegraph_bench pandagl)
  endif()
  if(HAVE_TINYDISPLAY)
    target_link_libraries(scenegraph_bench p3tinydisplay)
  endif()
endif()

find_package(Threads REQUIRED)
target_link_libraries(scenegraph_bench Threads::Threads)

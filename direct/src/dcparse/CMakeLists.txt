if(WIN32)
  # We can't build dcparse on Windows, because the dcparser symbols aren't
  # properly exported from p3direct.dll.
  return()
endif()

add_executable(dcparse dcparse.cxx)
target_compile_definitions(dcparse PUBLIC WITHIN_PANDA)
target_link_libraries(dcparse p3direct)
install(TARGETS dcparse DESTINATION bin)

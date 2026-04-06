if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(p3dcparse dcparse.cxx)
target_link_libraries(p3dcparse p3direct)
install(TARGETS p3dcparse EXPORT Direct COMPONENT Direct DESTINATION ${CMAKE_INSTALL_BINDIR})

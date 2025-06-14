if(NOT BUILD_PANDATOOL)
  # It's safe to say, if the user doesn't want pandatool, they don't want pview
  # either.
  return()
endif()

add_executable(pview pview.cxx)
target_link_libraries(pview p3framework)
install(TARGETS pview EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

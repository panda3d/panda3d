if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(pview pview.cxx)
target_link_libraries(pview p3framework)
install(TARGETS pview EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

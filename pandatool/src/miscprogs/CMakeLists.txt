if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(bin2c binToC.cxx binToC.h)
target_link_libraries(bin2c p3progbase)
install(TARGETS bin2c EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

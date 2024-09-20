if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(lwo-scan lwoScan.cxx lwoScan.h)
target_link_libraries(lwo-scan p3progbase p3lwo)
install(TARGETS lwo-scan EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

if(HAVE_EGG)

  add_executable(lwo2egg lwoToEgg.cxx lwoToEgg.h)
  target_link_libraries(lwo2egg p3lwoegg p3eggbase p3progbase)
  install(TARGETS lwo2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()

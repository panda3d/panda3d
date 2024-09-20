if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(x-trans xFileTrans.cxx xFileTrans.h)
target_link_libraries(x-trans p3progbase p3xfile)
install(TARGETS x-trans EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

if(HAVE_EGG)

  add_executable(egg2x eggToX.cxx eggToX.h)
  target_link_libraries(egg2x p3xfileegg p3eggbase p3progbase)
  install(TARGETS egg2x EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

  add_executable(x2egg xFileToEgg.cxx xFileToEgg.h)
  target_link_libraries(x2egg p3xfileegg p3eggbase p3progbase)
  install(TARGETS x2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()

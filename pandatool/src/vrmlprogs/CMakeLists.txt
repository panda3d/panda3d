if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(vrml-trans vrmlTrans.cxx vrmlTrans.h)
target_link_libraries(vrml-trans p3progbase p3vrml)
install(TARGETS vrml-trans EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

if(HAVE_EGG)

  add_executable(vrml2egg vrmlToEgg.cxx vrmlToEgg.h)
  target_link_libraries(vrml2egg p3vrmlegg p3eggbase p3progbase)
  install(TARGETS vrml2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()

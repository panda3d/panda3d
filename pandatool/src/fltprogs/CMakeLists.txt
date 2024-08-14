if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(flt-info fltInfo.cxx fltInfo.h)
target_link_libraries(flt-info p3progbase p3flt)
install(TARGETS flt-info EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(flt-trans fltTrans.cxx fltTrans.h)
target_link_libraries(flt-trans p3progbase p3flt)
install(TARGETS flt-trans EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

if(HAVE_EGG)

  add_executable(egg2flt eggToFlt.cxx eggToFlt.h)
  target_link_libraries(egg2flt p3fltegg p3eggbase p3progbase)
  install(TARGETS egg2flt EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

  add_executable(flt2egg fltToEgg.cxx fltToEgg.h)
  target_link_libraries(flt2egg p3fltegg p3eggbase p3progbase)
  install(TARGETS flt2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()

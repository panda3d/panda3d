if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT HAVE_EGG)
  return()
endif()

add_executable(egg2obj eggToObj.cxx eggToObj.h)
target_link_libraries(egg2obj p3objegg p3eggbase)
install(TARGETS egg2obj EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(obj2egg objToEgg.cxx objToEgg.h)
target_link_libraries(obj2egg p3objegg p3eggbase p3progbase)
install(TARGETS obj2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

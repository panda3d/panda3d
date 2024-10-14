if(NOT HAVE_EGG)
  return()
endif()

set(P3CONVERTER_HEADERS
  somethingToEggConverter.h somethingToEggConverter.I
  eggToSomethingConverter.h eggToSomethingConverter.I
)

set(P3CONVERTER_SOURCES
  somethingToEggConverter.cxx eggToSomethingConverter.cxx
)

add_library(p3converter STATIC ${P3CONVERTER_HEADERS} ${P3CONVERTER_SOURCES})
target_link_libraries(p3converter p3pandatoolbase pandaegg)

install(TARGETS p3converter
  EXPORT ToolsDevel COMPONENT ToolsDevel
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT ToolsDevel)
install(FILES ${P3CONVERTER_HEADERS} COMPONENT ToolsDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

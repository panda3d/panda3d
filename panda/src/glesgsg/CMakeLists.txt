if(NOT HAVE_GLES1)
  return()
endif()

set(P3GLESGSG_HEADERS
  config_glesgsg.h
  glesgsg.h
)

set(P3GLESGSG_SOURCES
  config_glesgsg.cxx
  glesgsg.cxx
)

composite_sources(p3glesgsg P3GLESGSG_SOURCES)
add_component_library(p3glesgsg SYMBOL BUILDING_PANDAGLES
  ${P3GLESGSG_HEADERS} ${P3GLESGSG_SOURCES})
target_link_libraries(p3glesgsg p3glstuff panda
  PKG::GLES1)

if(NOT BUILD_METALIBS)
  install(TARGETS p3glesgsg
    EXPORT OpenGLES1 COMPONENT OpenGLES1
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT OpenGLES1Devel)
endif()
install(FILES ${P3GLESGSG_HEADERS} COMPONENT OpenGLES1Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

set(P3PSTATCLIENT_HEADERS
  config_pstatclient.h pStatClient.I pStatClient.h
  pStatClientImpl.I pStatClientImpl.h
  pStatClientVersion.I
  pStatClientVersion.h pStatClientControlMessage.h
  pStatCollector.I pStatCollector.h pStatCollectorDef.h
  pStatCollectorForward.I pStatCollectorForward.h
  pStatFrameData.I pStatFrameData.h pStatProperties.h
  pStatServerControlMessage.h pStatThread.I pStatThread.h
  pStatTimer.I pStatTimer.h
)

set(P3PSTATCLIENT_SOURCES
  config_pstatclient.cxx pStatClient.cxx pStatClientImpl.cxx
  pStatClientVersion.cxx
  pStatClientControlMessage.cxx
  pStatCollector.cxx
  pStatCollectorDef.cxx
  pStatCollectorForward.cxx
  pStatFrameData.cxx pStatProperties.cxx
  pStatServerControlMessage.cxx
  pStatThread.cxx
)

set(P3PSTATCLIENT_IGATEEXT
  pStatClient_ext.I
  pStatClient_ext.cxx
  pStatClient_ext.h
)

composite_sources(p3pstatclient P3PSTATCLIENT_SOURCES)
add_component_library(p3pstatclient SYMBOL BUILDING_PANDA_PSTATCLIENT
  ${P3PSTATCLIENT_HEADERS} ${P3PSTATCLIENT_SOURCES})
target_link_libraries(p3pstatclient p3putil pandaexpress)

if(HAVE_NET AND WANT_NATIVE_NET)
  target_link_libraries(p3pstatclient p3net)
endif()

target_interrogate(p3pstatclient ALL EXTENSIONS ${P3PSTATCLIENT_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3pstatclient
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PSTATCLIENT_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
install(FILES config_pstatclient.h COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

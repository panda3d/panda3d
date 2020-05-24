set(P3RECORDER_HEADERS
  config_recorder.h
  mouseRecorder.h
  recorderBase.h recorderBase.I
  recorderController.h recorderController.I
  recorderFrame.h recorderFrame.I
  recorderHeader.h recorderHeader.I
  recorderTable.h recorderTable.I
  socketStreamRecorder.h socketStreamRecorder.I
)

set(P3RECORDER_SOURCES
  config_recorder.cxx
  mouseRecorder.cxx
  recorderBase.cxx recorderController.cxx
  recorderFrame.cxx recorderHeader.cxx recorderTable.cxx
  socketStreamRecorder.cxx
)

composite_sources(p3recorder P3RECORDER_SOURCES)
add_component_library(p3recorder NOINIT SYMBOL BUILDING_PANDA_RECORDER
  ${P3RECORDER_HEADERS} ${P3RECORDER_SOURCES})
target_link_libraries(p3recorder p3dgraph pandaexpress)
target_interrogate(p3recorder ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3recorder
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3RECORDER_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

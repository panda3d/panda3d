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
add_library(p3recorder ${P3RECORDER_HEADERS} ${P3RECORDER_SOURCES})
set_target_properties(p3recorder PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_RECORDER)
target_link_libraries(p3recorder p3dgraph p3downloader)
target_interrogate(p3recorder ALL)

install(TARGETS p3recorder DESTINATION lib)
install(FILES ${P3RECORDER_HEADERS} DESTINATION include/panda3d)

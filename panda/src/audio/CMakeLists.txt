if(NOT HAVE_AUDIO)
  return()
endif()

set(P3AUDIO_HEADERS
  config_audio.h
  filterProperties.h filterProperties.I
  audioLoadRequest.h audioLoadRequest.I
  audioManager.h audioManager.I
  audioSound.h audioSound.I
  nullAudioManager.h
  nullAudioSound.h
)

set(P3AUDIO_SOURCES
  config_audio.cxx
  filterProperties.cxx
  audioLoadRequest.cxx
  audioManager.cxx
  audioSound.cxx
  nullAudioManager.cxx
  nullAudioSound.cxx
)

composite_sources(p3audio P3AUDIO_SOURCES)
add_component_library(p3audio NOINIT SYMBOL BUILDING_PANDA_AUDIO
  ${P3AUDIO_HEADERS} ${P3AUDIO_SOURCES})
target_link_libraries(p3audio p3putil p3event p3movies p3linmath)
target_interrogate(p3audio ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3audio
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3AUDIO_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

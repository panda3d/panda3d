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
    nullAudioSound.h)

set(P3AUDIO_SOURCES
    config_audio.cxx
    filterProperties.cxx
    audioLoadRequest.cxx
    audioManager.cxx
    audioSound.cxx
    nullAudioManager.cxx
    nullAudioSound.cxx)

composite_sources(p3audio P3AUDIO_SOURCES)
add_component_library(p3audio SYMBOL BUILDING_PANDA_AUDIO
  ${P3AUDIO_HEADERS} ${P3AUDIO_SOURCES})
target_link_libraries(p3audio p3putil p3event p3movies p3linmath)
target_interrogate(p3audio ALL)

install(TARGETS p3audio DESTINATION lib)
install(FILES ${P3AUDIO_HEADERS} DESTINATION include/panda3d)

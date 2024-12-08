if(NOT HAVE_AUDIO)
  return()
elseif(NOT HAVE_OPENAL)
  message(SEND_ERROR
    "You must have an OpenAL for steam audio support! Turn off HAVE_AUDIO to ignore this.")
endif()

if(HAVE_STEAMAUDIO)
  set(P3STEAMAUDIO_HEADERS
    config_steamaudio.h
    steamAudioManager.h
    steamAudioSound.I steamAudioSound.h
    steamAudioEffect.h
    steamDirectEffect.h
  )

  set(P3STEAMAUDIO_SOURCES
    config_steamaudio.cxx steamAudioManager.cxx steamAudioSound.cxx steamAudioEffect.cxx steamDirectEffect.cxx
  )


  composite_sources(p3steam_audio P3STEAMAUDIO_SOURCES)
  add_library(p3steam_audio ${MODULE_TYPE} ${P3STEAMAUDIO_HEADERS} ${P3STEAMAUDIO_SOURCES})
  set_target_properties(p3steam_audio PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_STEAMAUDIO)
  target_link_libraries(p3steam_audio panda)
  message(DEBUG "Passed where Building steam audio should be defined")#DELETE THIS BEFORE COMMIT
  if(HAVE_OPENAL_FRAMEWORK)
    target_link_libraries(p3steam_audio "-framework OpenAL")
  else()
    target_link_libraries(p3steam_audio PKG::OPENAL)
  endif()
  target_link_libraries(p3steam_audio "-framework SteamAudio")

  if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
    # When statically linking OpenAL, keep its symbols private to this module.
    target_link_options(p3steam_audio PRIVATE "LINKER:--exclude-libs,libopenal.a")
    #Do I need to do this to steam audio as well? Does steam audio using openal change this?
  endif()

  install(TARGETS p3steam_audio
    EXPORT SteamAudio COMPONENT SteamAudio
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT SteamAudioDevel)
  install(FILES ${P3STEAMAUDIO_HEADERS} COMPONENT SteamAudioDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

  export_targets(SteamAudio NAMESPACE "Panda3D::SteamAudio::" COMPONENT SteamAudioDevel)
endif()

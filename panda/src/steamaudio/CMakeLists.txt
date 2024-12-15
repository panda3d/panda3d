if(HAVE_STEAMAUDIO)
  set(P3STEAMAUDIO_HEADERS
    config_steamaudio.h
    steamMovieAudio.h
    steamMovieAudioCursor.h
    steamAudioEffect.h
    steamDirectEffect.h
  )

  set(P3STEAMAUDIO_SOURCES
    config_steamaudio.cxx steamMovieAudio.cxx steamMovieAudioCursor.cxx steamAudioEffect.cxx steamDirectEffect.cxx
  )


  composite_sources(p3steam_audio P3STEAMAUDIO_SOURCES)
  add_library(p3steam_audio ${MODULE_TYPE} ${P3STEAMAUDIO_HEADERS} ${P3STEAMAUDIO_SOURCES})
  set_target_properties(p3steam_audio PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_STEAMAUDIO)
  target_link_libraries(p3steam_audio panda)

  target_link_libraries(p3steam_audio "-framework SteamAudio")

  install(TARGETS p3steam_audio
    EXPORT SteamAudio COMPONENT SteamAudio
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT SteamAudioDevel)
  install(FILES ${P3STEAMAUDIO_HEADERS} COMPONENT SteamAudioDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

  export_targets(SteamAudio NAMESPACE "Panda3D::SteamAudio::" COMPONENT SteamAudioDevel)
endif()

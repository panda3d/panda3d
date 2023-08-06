if(NOT HAVE_AUDIO)
  return()
elseif(NOT HAVE_FMODEX AND NOT HAVE_OPENAL)
  message(SEND_ERROR
    "You must have an audio backend for audio support! Turn off HAVE_AUDIO to ignore this.")
endif()

if(HAVE_FMODEX AND NOT (APPLE AND CMAKE_OSX_ARCHITECTURES STREQUAL "arm64"))
  set(P3FMOD_HEADERS
    config_fmodAudio.h
    fmodAudioManager.h
    fmodAudioSound.I fmodAudioSound.h
  )

  set(P3FMOD_SOURCES
    config_fmodAudio.cxx fmodAudioManager.cxx fmodAudioSound.cxx
  )

  composite_sources(p3fmod_audio P3FMOD_SOURCES)
  add_library(p3fmod_audio ${MODULE_TYPE} ${P3FMOD_HEADERS} ${P3FMOD_SOURCES})
  set_target_properties(p3fmod_audio PROPERTIES DEFINE_SYMBOL BUILDING_FMOD_AUDIO)
  target_link_libraries(p3fmod_audio panda PKG::FMODEX)

  if(APPLE)
    # The FMOD EX thirdparty library is not available for arm64, so we have to
    # exclude it from the target architectures.
    get_target_property(_archs p3fmod_audio OSX_ARCHITECTURES)

    if(_archs AND _archs MATCHES "arm64")
      list(REMOVE_ITEM _archs "arm64")
      set_target_properties(p3fmod_audio PROPERTIES OSX_ARCHITECTURES "${_archs}")
    endif()
  endif()

  install(TARGETS p3fmod_audio
    EXPORT FMOD COMPONENT FMOD
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT FMODDevel)
  install(FILES ${P3FMOD_HEADERS} COMPONENT FMODDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

  export_targets(FMOD NAMESPACE "Panda3D::FMOD::" COMPONENT FMODDevel)
endif()

if(HAVE_OPENAL)
  set(P3OPENAL_HEADERS
    config_openalAudio.h
    openalAudioManager.h
    openalAudioSound.I openalAudioSound.h
  )

  set(P3OPENAL_SOURCES
    config_openalAudio.cxx openalAudioManager.cxx openalAudioSound.cxx
  )


  composite_sources(p3openal_audio P3OPENAL_SOURCES)
  add_library(p3openal_audio ${MODULE_TYPE} ${P3OPENAL_HEADERS} ${P3OPENAL_SOURCES})
  set_target_properties(p3openal_audio PROPERTIES DEFINE_SYMBOL BUILDING_OPENAL_AUDIO)
  target_link_libraries(p3openal_audio panda)

  if(HAVE_OPENAL_FRAMEWORK)
    target_link_libraries(p3openal_audio "-framework OpenAL")
  else()
    target_link_libraries(p3openal_audio PKG::OPENAL)
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
    # When statically linking OpenAL, keep its symbols private to this module.
    target_link_options(p3openal_audio PRIVATE "LINKER:--exclude-libs,libopenal.a")
  endif()

  install(TARGETS p3openal_audio
    EXPORT OpenAL COMPONENT OpenAL
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT OpenALDevel)
  install(FILES ${P3OPENAL_HEADERS} COMPONENT OpenALDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

  export_targets(OpenAL NAMESPACE "Panda3D::OpenAL::" COMPONENT OpenALDevel)
endif()

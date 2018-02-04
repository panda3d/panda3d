if(HAVE_AUDIO)

  if(HAVE_RAD_MSS)
    set(P3MILES_HEADERS
        config_milesAudio.h
        milesAudioManager.h
        milesAudioSound.I milesAudioSound.h
        milesAudioSample.I milesAudioSample.h
        milesAudioSequence.I milesAudioSequence.h
        milesAudioStream.I milesAudioStream.h
        globalMilesManager.I globalMilesManager.h)

    set(P3MILES_SOURCES 
        config_milesAudio.cxx milesAudioManager.cxx milesAudioSound.cxx
        milesAudioStream.cxx globalMilesManager.cxx milesAudioSample.cxx
        milesAudioSequence.cxx)

    composite_sources(p3miles_audio P3MILES_SOURCES)
    include_directories(${MILES_INCLUDE_DIR})
    add_library(p3miles_audio ${P3MILES_HEADERS} ${P3MILES_SOURCES})
    target_link_libraries(p3miles_audio p3audio p3event p3pipeline)
    target_use_packages(p3miles_audio MILES)

    install(TARGETS p3miles_audio DESTINATION lib)
  endif()

  if(HAVE_FMODEX)
    set(P3FMOD_HEADERS
        config_fmodAudio.h
        fmodAudioManager.h
        fmodAudioSound.I fmodAudioSound.h)

    set(P3FMOD_SOURCES      
        config_fmodAudio.cxx fmodAudioManager.cxx fmodAudioSound.cxx)

    composite_sources(p3fmod_audio P3FMOD_SOURCES)
    include_directories(${FMODEX_INCLUDE_DIR})
    add_library(p3fmod_audio ${P3FMOD_HEADERS} ${P3FMOD_SOURCES})
    target_link_libraries(p3fmod_audio p3audio p3event)
    target_use_packages(p3fmod_audio FMODEX)

    install(TARGETS p3fmod_audio DESTINATION lib)
  endif()

  if(HAVE_OPENAL)
    set(P3OPENAL_HEADERS
        config_openalAudio.h
        openalAudioManager.h
        openalAudioSound.I openalAudioSound.h)

    set(P3OPENAL_SOURCES
        config_openalAudio.cxx openalAudioManager.cxx openalAudioSound.cxx)


    composite_sources(p3openal_audio P3OPENAL_SOURCES)
    include_directories(${OPENAL_INCLUDE_DIR})
    add_library(p3openal_audio ${P3OPENAL_HEADERS} ${P3OPENAL_SOURCES})
    target_link_libraries(p3openal_audio p3audio p3event)
    target_use_packages(p3openal_audio OPENAL)

    install(TARGETS p3openal_audio DESTINATION lib)
  endif()

endif()

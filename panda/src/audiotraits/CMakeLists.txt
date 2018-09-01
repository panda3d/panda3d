if(NOT HAVE_AUDIO)
  return()
elseif(NOT HAVE_RAD_MSS AND NOT HAVE_FMODEX AND NOT HAVE_OPENAL)
  message(SEND_ERROR
    "You must have an audio backend for audio support! Turn off HAVE_AUDIO to ignore this.")
endif()

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
  set_target_properties(p3miles_audio PROPERTIES DEFINE_SYMBOL BUILDING_MILES_AUDIO)
  target_link_libraries(p3miles_audio panda)
  target_use_packages(p3miles_audio MILES)

  install(TARGETS p3miles_audio DESTINATION lib RUNTIME DESTINATION bin)
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
  set_target_properties(p3fmod_audio PROPERTIES DEFINE_SYMBOL BUILDING_FMOD_AUDIO)
  target_link_libraries(p3fmod_audio panda)
  target_use_packages(p3fmod_audio FMODEX)

  install(TARGETS p3fmod_audio DESTINATION lib RUNTIME DESTINATION bin)
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
  set_target_properties(p3openal_audio PROPERTIES DEFINE_SYMBOL BUILDING_OPENAL_AUDIO)
  target_link_libraries(p3openal_audio panda)
  target_use_packages(p3openal_audio OPENAL)

  install(TARGETS p3openal_audio DESTINATION lib RUNTIME DESTINATION bin)
endif()

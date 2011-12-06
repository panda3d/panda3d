#define OTHER_LIBS p3dtoolconfig p3dtool p3dtoolbase:c p3dtoolutil:c \
    p3putil:c p3prc:c p3interrogatedb:c p3dconfig:c

#define BUILD_DIRECTORY $[HAVE_AUDIO]

#begin lib_target
  #define TARGET miles_audio
  #define BUILD_TARGET $[HAVE_RAD_MSS]
  #define USE_PACKAGES rad_mss
  #define BUILDING_DLL BUILDING_MILES_AUDIO
  #define LOCAL_LIBS p3audio p3event p3pipeline
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
      config_milesAudio.h \
      milesAudioManager.h \
      milesAudioSound.I milesAudioSound.h \
      milesAudioSample.I milesAudioSample.h \
      milesAudioSequence.I milesAudioSequence.h \
      milesAudioStream.I milesAudioStream.h \
      globalMilesManager.I globalMilesManager.h

  #define INCLUDED_SOURCES \
      config_milesAudio.cxx milesAudioManager.cxx milesAudioSound.cxx \
      milesAudioStream.cxx globalMilesManager.cxx milesAudioSample.cxx \
      milesAudioSequence.cxx

#end lib_target

#begin lib_target
  #define TARGET p3fmod_audio
  #define BUILD_TARGET $[HAVE_FMODEX]
  #define USE_PACKAGES fmodex
  #define BUILDING_DLL BUILDING_FMOD_AUDIO
  #define LOCAL_LIBS p3audio p3event
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #define COMBINED_SOURCES fmod_audio_composite1.cxx

  #define SOURCES \
      config_fmodAudio.h \
      fmodAudioManager.h \
      fmodAudioSound.I fmodAudioSound.h

  #define INCLUDED_SOURCES \
      config_fmodAudio.cxx fmodAudioManager.cxx fmodAudioSound.cxx

#end lib_target

#begin lib_target
  #define TARGET p3openal_audio
  #define BUILD_TARGET $[HAVE_OPENAL]
  #define USE_PACKAGES openal
  #define BUILDING_DLL BUILDING_OPENAL_AUDIO
  #define LOCAL_LIBS p3audio p3event
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #define COMBINED_SOURCES openal_audio_composite1.cxx

  #define SOURCES \
      config_openalAudio.h \
      openalAudioManager.h \
      openalAudioSound.I openalAudioSound.h

  #define INCLUDED_SOURCES \
      config_openalAudio.cxx openalAudioManager.cxx openalAudioSound.cxx

#end lib_target

//#begin lib_target
//  #define TARGET audio_linux
//  #define BUILDING_DLL BUILDING_MISC
//  //#define LOCAL_LIBS \
//  //  p3audio
//
//  #define SOURCES \
//    $[if $[HAVE_SYS_SOUNDCARD_H], \
//      linuxAudioManager.cxx linuxAudioManager.h \
//      linuxAudioSound.cxx linuxAudioSound.h \
//    , \
//      nullAudioManager.cxx nullAudioManager.h \
//      nullAudioSound.cxx nullAudioSound.h \
//    ]
//
//#end lib_target

#define OTHER_LIBS dtoolconfig dtool
#define BUILD_DIRECTORY $[HAVE_AUDIO]

#begin lib_target
  #define TARGET miles_audio
  #define BUILD_TARGET $[HAVE_RAD_MSS]
  #define USE_PACKAGES rad_mss
  #define BUILDING_DLL BUILDING_MILES_AUDIO
  #define LOCAL_LIBS audio
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  

  #define SOURCES \
      config_milesAudio.h \
      milesAudioManager.h \
      milesAudioSound.I milesAudioSound.h
      
  #define INCLUDED_SOURCES \
      config_milesAudio.cxx milesAudioManager.cxx milesAudioSound.cxx 

#end lib_target

#begin lib_target
  #define TARGET fmod_audio
  #define BUILD_TARGET $[HAVE_FMOD]
  #define USE_PACKAGES fmod
  #define BUILDING_DLL BUILDING_FMOD_AUDIO
  #define LOCAL_LIBS audio
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
      config_fmodAudio.h \
      fmodAudioManager.h \
      fmodAudioSound.I fmodAudioSound.h

  #define INCLUDED_SOURCES \
      config_fmodAudio.cxx fmodAudioManager.cxx fmodAudioSound.cxx

#end lib_target

//#begin lib_target
//  #define TARGET audio_linux
//  #define BUILDING_DLL BUILDING_MISC
//  //#define LOCAL_LIBS \
//  //  audio
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

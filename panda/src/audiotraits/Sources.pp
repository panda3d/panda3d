#define OTHER_LIBS dtoolconfig dtool
#define DIRECTORY_IF_AUDIO yes

#begin lib_target
  #define USE_RAD_MSS yes
  #define TARGET miles_audio
  #define BUILDING_DLL BUILDING_MILES_AUDIO
  #define LOCAL_LIBS audio
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] Mss32.lib user32.lib advapi32.lib
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  

  #define SOURCES \
      config_milesAudio.h \
      milesAudioManager.h \
      milesAudioSound.I milesAudioSound.h
      
  #define INCLUDED_SOURCES \
      config_milesAudio.cxx milesAudioManager.cxx milesAudioSound.cxx 

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

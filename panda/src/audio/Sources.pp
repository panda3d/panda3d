#define OTHER_LIBS dtoolconfig dtool
#define DIRECTORY_IF_AUDIO yes
#define DIRECTORY_IF_IPC yes
// #define USE_MIKMOD yes

#begin lib_target
  #define USE_AUDIO yes
  #define USE_MIKMOD yes

  #define TARGET audio
  #define LOCAL_LIBS putil ipc gui

  #define SOURCES \
    config_audio.cxx config_audio.h \
    audioManager.I audioManager.cxx audioManager.h \
    audioSound.cxx audioSound.h \
    $[if $[HAVE_SYS_SOUNDCARD_H], \
      linuxAudioManager.cxx linuxAudioManager.h \
      linuxAudioSound.cxx linuxAudioSound.h \
    ,] \
    $[if $[HAVE_RAD_MSS], \
      milesAudioManager.cxx milesAudioManager.h \
      milesAudioSound.I milesAudioSound.cxx milesAudioSound.h \
    ,] \
    nullAudioManager.cxx nullAudioManager.h \
    nullAudioSound.cxx nullAudioSound.h \
    audio_gui_functor.h audio_gui_functor.cxx

  #define INSTALL_HEADERS \
    config_audio.h \
    audio.h \
    audioManager.h audioManager.I \
    audioSound.h \
    $[if $[HAVE_SYS_SOUNDCARD_H], \
      linuxAudioManager.h \
      linuxAudioSound.h \
    ,] \
    $[if $[HAVE_RAD_MSS], \
      milesAudioManager.h \
      milesAudioSound.h milesAudioSound.I \
    ,] \
    nullAudioManager.h \
    nullAudioSound.h \
    audio_gui_functor.h

  #define IGATESCAN audio.h
#end lib_target

#begin test_bin_target
  #define TARGET test_audio
  #define LOCAL_LIBS \
    audio
  #define OTHER_LIBS \
    $[OTHER_LIBS] pystub

  #define SOURCES \
    test_audio.cxx

#end test_bin_target


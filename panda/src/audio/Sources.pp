#define OTHER_LIBS dtoolconfig dtool
#define DIRECTORY_IF_AUDIO yes

#begin lib_target
  #define TARGET audio
  #define LOCAL_LIBS putil

  #define SOURCES \
    config_audio.cxx config_audio.h \
    audioManager.I audioManager.cxx audioManager.h \
    audioSound.cxx audioSound.h \
    nullAudioManager.cxx nullAudioManager.h \
    nullAudioSound.cxx nullAudioSound.h

  #define INSTALL_HEADERS \
    config_audio.h \
    audio.h \
    audioManager.h audioManager.I \
    audioSound.h \
    nullAudioManager.h \
    nullAudioSound.h \

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


#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
     dtoolutil:c dtoolbase:c dtool:m
#define BUILD_DIRECTORY $[HAVE_AUDIO]

#begin lib_target
  #define TARGET audio
  #define LOCAL_LIBS putil
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_audio.h audioManager.h \
    audioSound.h nullAudioManager.h nullAudioSound.h
    
  #define INCLUDED_SOURCES \
    config_audio.cxx audioManager.cxx audioSound.cxx \
    nullAudioManager.cxx nullAudioSound.cxx

  #define INSTALL_HEADERS \
    config_audio.h \
    audio.h \
    audioManager.h \
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

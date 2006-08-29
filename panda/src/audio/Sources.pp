#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
     dtoolutil:c dtoolbase:c dtool:m prc:c
#define BUILD_DIRECTORY $[HAVE_AUDIO]

#begin lib_target
  #define TARGET audio
  #define LOCAL_LIBS putil event
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_audio.h \
    audioDSP.h \
    audioLoadRequest.h audioLoadRequest.I \
    audioManager.h \
    audioSound.h \
    nullAudioDSP.h \
    nullAudioManager.h \
    nullAudioSound.h
    
  #define INCLUDED_SOURCES \
    config_audio.cxx \
    audioDSP.cxx \
    audioLoadRequest.cxx \
    audioManager.cxx \
    audioSound.cxx \
    nullAudioDSP.cxx \
    nullAudioManager.cxx \
    nullAudioSound.cxx

  #define INSTALL_HEADERS \
    config_audio.h \
    audioDSP.h \
    audioLoadRequest.h audioLoadRequest.I \
    audioManager.h \
    audioSound.h \
    nullAudioDSP.h \
    nullAudioManager.h \
    nullAudioSound.h

  #define IGATESCAN all
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

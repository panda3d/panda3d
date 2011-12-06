#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
     p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define BUILD_DIRECTORY $[HAVE_AUDIO]

#begin lib_target
  #define TARGET p3audio
  #define LOCAL_LIBS p3putil p3event p3movies p3linmath
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_audio.h \
    filterProperties.h filterProperties.I\
    audioLoadRequest.h audioLoadRequest.I \
    audioManager.h audioManager.I\
    audioSound.h audioSound.I\
    nullAudioManager.h \
    nullAudioSound.h
    
  #define INCLUDED_SOURCES \
    config_audio.cxx \
    filterProperties.cxx \
    audioLoadRequest.cxx \
    audioManager.cxx \
    audioSound.cxx \
    nullAudioManager.cxx \
    nullAudioSound.cxx

  #define INSTALL_HEADERS \
    config_audio.h \
    filterProperties.h filterProperties.I\
    audioLoadRequest.h audioLoadRequest.I \
    audioManager.h audioManager.I\
    audioSound.h audioSound.I\
    nullAudioManager.h \
    nullAudioSound.h

  #define IGATESCAN all
#end lib_target

#begin test_bin_target
  #define TARGET test_audio
  #define LOCAL_LIBS \
    p3audio
  #define OTHER_LIBS \
    $[OTHER_LIBS] p3pystub

  #define SOURCES \
    test_audio.cxx

#end test_bin_target

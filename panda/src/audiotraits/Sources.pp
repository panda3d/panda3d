#define OTHER_LIBS dtool
#define DIRECTORY_IF_AUDIO yes

#begin lib_target
  #define TARGET audio_load_midi
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    audio_load_midi.cxx

#end lib_target

#begin lib_target
  #define TARGET audio_load_wav
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    audio_load_wav.cxx

#end lib_target

#begin lib_target
  #define TARGET audio_load_st
  #define BUILDING_DLL BUILDING_MISC
  #define USE_SOXST yes
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    audio_load_st.cxx

#end lib_target

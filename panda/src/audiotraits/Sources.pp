#define OTHER_LIBS dtoolconfig dtool
#define DIRECTORY_IF_AUDIO yes
#define DIRECTORY_IF_IPC yes
#define USE_AUDIO yes
#define USE_RAD_MSS yes

#begin lib_target
  #define TARGET audio_load_midi
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    audio express

  #define SOURCES \
    audio_load_midi.cxx

#end lib_target

#begin lib_target
  #define TARGET audio_load_wav
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    audio express

  #define SOURCES \
    audio_load_wav.cxx

#end lib_target

#begin lib_target
  #define TARGET audio_load_st
  #define BUILDING_DLL BUILDING_MISC
  #define USE_SOXST yes
  #define LOCAL_LIBS \
    audio express

  #define SOURCES \
    audio_load_st.cxx

#end lib_target

#begin lib_target
  #define TARGET audio_load_mp3
  #define BUILDING_DLL BUILDING_MISC
  #define LOCAL_LIBS \
    audio mpg123 express
  #define C++FLAGS -DGENERIC -DNOXFERMEM

  #define SOURCES \
    audio_load_mp3.cxx

#end lib_target

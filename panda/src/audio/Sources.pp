#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET audio
  #define LOCAL_LIBS putil ipc

  #define SOURCES \
    audio_manager.I audio_manager.cxx audio_manager.h audio_midi.cxx \
    audio_midi.h audio_music.I audio_music.cxx audio_music.h \
    audio_pool.I audio_pool.cxx audio_pool.h audio_sample.I \
    audio_sample.cxx audio_sample.h audio_trait.cxx audio_trait.h \
    config_audio.cxx config_audio.h

  #define INSTALL_HEADERS \
    audio.h audio_manager.h audio_music.h audio_pool.I audio_pool.h \
    audio_sample.I audio_sample.h audio_trait.h

  #define IGATESCAN audio.h

#end lib_target

#begin lib_target
  #define TARGET audio_load_midi
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    audio_load_midi.cxx

#end lib_target

#begin lib_target
  #define TARGET audio_load_wav
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    audio_load_wav.cxx

#end lib_target

#begin test_bin_target
  #define TARGET test_audio

  #define SOURCES \
    test_audio.cxx

#end test_bin_target


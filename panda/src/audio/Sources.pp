#define OTHER_LIBS dtool

#begin lib_target
  #define USE_AUDIO yes

  #define TARGET audio
  #define LOCAL_LIBS putil ipc

  #define SOURCES \
    audio_manager.I audio_manager.cxx audio_manager.h audio_midi.cxx \
    audio_midi.h audio_music.I audio_music.cxx audio_music.h \
    audio_pool.I audio_pool.cxx audio_pool.h audio_sample.I \
    audio_sample.cxx audio_sample.h audio_trait.cxx audio_trait.h \
    config_audio.cxx config_audio.h audio_mikmod_traits.cxx \
    audio_mikmod_traits.h audio_win_traits.I audio_win_traits.cxx \
    audio_win_traits.h audio_null_traits.I audio_null_traits.cxx \
    audio_null_traits.h audio_linux_traits.I audio_linux_traits.cxx \
    audio_linux_traits.h

  #define INSTALL_HEADERS \
    audio.h audio_manager.h audio_music.h audio_pool.I audio_pool.h \
    audio_sample.I audio_sample.h audio_trait.h audio_mikmod_traits.h \
    audio_win_traits.I audio_win_traits.h audio_null_traits.I \
    audio_null_traits.h audio_linux_traits.I audio_linux_traits.h

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

#begin lib_target
  #define TARGET audio_load_st
  #define USE_SOXST yes
  #define LOCAL_LIBS \
    audio

  #define SOURCES \
    audio_load_st.cxx

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


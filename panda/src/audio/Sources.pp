#define OTHER_LIBS dtoolconfig dtool
#define DIRECTORY_IF_AUDIO yes
#define DIRECTORY_IF_IPC yes
// #define USE_MIKMOD yes

#begin lib_target
  #define USE_AUDIO yes
  #define USE_MIKMOD yes
  #define USE_RAD_MSS yes

  #define TARGET audio
  #define LOCAL_LIBS putil ipc gui

  #define SOURCES \
    audio_manager.I audio_manager.cxx audio_manager.h \
    audio_midi.cxx audio_midi.h \
    audio_pool.I audio_pool.cxx audio_pool.h \
    audio_trait.cxx audio_trait.h \
    config_audio.cxx config_audio.h \
    audio_mikmod_traits.cxx audio_mikmod_traits.h \
    audio_win_traits.I audio_win_traits.cxx audio_win_traits.h \
    audio_null_traits.I audio_null_traits.cxx audio_null_traits.h \
    audio_rad_mss_traits.I audio_rad_mss_traits.cxx audio_rad_mss_traits.h \
    audio_linux_traits.I audio_linux_traits.cxx audio_linux_traits.h \
    audio_sound.I audio_sound.cxx audio_sound.h \
    audio_gui_functor.h audio_gui_functor.cxx

  #define INSTALL_HEADERS \
    audio.h audio_manager.h \
    audio_pool.I audio_pool.h \
    audio_trait.h audio_mikmod_traits.h \
    audio_win_traits.I audio_win_traits.h \
    audio_rad_mss_traits.I audio_rad_mss_traits.h \
    audio_null_traits.I audio_null_traits.h \
    audio_linux_traits.I audio_linux_traits.h \
    config_audio.h audio_manager.I audio_sound.h audio_sound.I \
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


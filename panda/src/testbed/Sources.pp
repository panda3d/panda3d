#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m pystub
#define LOCAL_LIBS \
    framework putil collide loader sgmanip chan text chancfg cull \
    pnmimage pnmimagetypes event effects shader graph gobj display \
    mathutil sgattrib putil express light dgraph device tform sgraph \
    linmath pstatclient sgraphutil
#define UNIX_SYS_LIBS m


#begin bin_target
  #define TARGET demo

  #define SOURCES \
    demo.cxx

#end bin_target

#begin test_bin_target
  #define TARGET chat
  #define LOCAL_LIBS $[LOCAL_LIBS] chat

  #define SOURCES \
    chat_test.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET downloader

  #define SOURCES \
    downloader_test.cxx

  #define USE_ZLIB yes
  #define TARGET_IF_ZLIB yes

#end test_bin_target

#begin test_bin_target
  #define TARGET herc

  #define SOURCES \
    herc.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET loader

  #define SOURCES \
    loader_test.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET lod

  #define SOURCES \
    lod_test.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET min_herc

  #define SOURCES \
    min_herc.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET min_shader

  #define SOURCES \
    min_shader.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET panda

  #define SOURCES \
    panda.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET shader

  #define SOURCES \
    shader_test.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_eggWrite

  #define SOURCES \
    test_eggWrite.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET test_particles

  #define SOURCES \
    test_particles.cxx
  #define LOCAL_LIBS $[LOCAL_LIBS] physics particlesystem

#end test_bin_target

#begin test_bin_target
  #define TARGET test_recparticles

  #define SOURCES \
    test_recparticles.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET text

  #define SOURCES \
    text_test.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET vrpn_demo

  #define SOURCES \
    vrpn_demo.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET gui_demo

  #define SOURCES \
    gui_demo.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] gui

#end test_bin_target

#begin test_bin_target
  #define TARGET deadrec_rec

  #define SOURCES \
    deadrec_rec.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] net gui
#end test_bin_target

#begin test_bin_target
  #define TARGET deadrec_send

  #define SOURCES \
    deadrec_send.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] net lerp gui
#end test_bin_target

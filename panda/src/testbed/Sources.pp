#define OTHER_LIBS dtool pystub
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

  #define SOURCES \
    chat_test.cxx

#end test_bin_target

#begin test_bin_target
  #define TARGET downloader

  #define SOURCES \
    downloader_test.cxx

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


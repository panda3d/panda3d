#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m pystub

#define LOCAL_LIBS \
    framework putil collide loader sgmanip chan text chancfg cull \
    pnmimage pnmimagetypes event effects graph gobj display \
    mathutil sgattrib putil express light dgraph device tform sgraph \
    linmath sgraphutil pstatclient panda

#if $[LINK_ALL_STATIC]
  // If we're statically linking, we need to explicitly link with
  // at least one graphics renderer.
  #define LOCAL_LIBS pandagl pandadx $[LOCAL_LIBS]

  // And we might like to have the egg loader available.
  #define LOCAL_LIBS pandaegg $[LOCAL_LIBS]
#endif

#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET demo

  #define SOURCES \
    demo.cxx

#end bin_target

#begin test_bin_target
  #define TARGET pview

  #define SOURCES \
    pview.cxx

  #define LOCAL_LIBS pgraph $[LOCAL_LIBS]
#end test_bin_target

#begin test_bin_target
  #define TARGET open_window

  #define SOURCES \
    open_window.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] pandagl pandadx
#end test_bin_target

#if $[HAVE_DX]
  #begin test_bin_target
    #define TARGET demo_multimon

    #define SOURCES \
      demo.cxx

    #define LOCAL_LIBS $[LOCAL_LIBS] pandadx framework_multimon
  #end test_bin_target
#endif

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
  #define TARGET test_sprite_particles

  #define SOURCES \
    test_sprite_particles.cxx
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

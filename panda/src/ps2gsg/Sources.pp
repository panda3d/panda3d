#define DIRECTORY_IF_PS2 yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET ps2gsg
  #define LOCAL_LIBS \
    cull gsgmisc gsgbase gobj sgattrib sgraphutil graph display light \
    putil linmath sgraph mathutil pnmimage

  #define SOURCES \
    config_ps2gsg.cxx ps2GraphicsStateGuardian.cxx \
    ps2SavedFrameBuffer.cxx ps2TextureContext.cxx

  #define INSTALL_HEADERS \

#end lib_target


#define DIRECTORY_IF_PS2 yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET ps2gsg
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define SOURCES \
    config_ps2gsg.cxx ps2GraphicsStateGuardian.cxx \
    ps2SavedFrameBuffer.cxx ps2TextureContext.cxx

  #define INSTALL_HEADERS \

#end lib_target


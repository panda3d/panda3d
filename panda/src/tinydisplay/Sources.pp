#define BUILD_DIRECTORY $[HAVE_TINYSDGL]
//#define BUILDING_DLL BUILDING_PANDATINYSDGL

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES tinysdgl

#begin lib_target
  #define TARGET tinydisplay
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define SOURCES \
    config_tinydisplay.cxx config_tinydisplay.h \
    tinyGeomMunger.I tinyGeomMunger.cxx tinyGeomMunger.h \
    tinyGraphicsPipe.I tinyGraphicsPipe.cxx tinyGraphicsPipe.h \
    tinyGraphicsWindow.h tinyGraphicsWindow.I tinyGraphicsWindow.cxx \
    tinyGraphicsStateGuardian.h tinyGraphicsStateGuardian.I \
    tinyGraphicsStateGuardian.cxx \
    tinyImmediateModeSender.h tinyImmediateModeSender.I \
    tinyImmediateModeSender.cxx \
    tinyTextureContext.I tinyTextureContext.cxx tinyTextureContext.h \
    api.c arrays.c clear.c clip.c error.c get.c \
    glu.c image_util.c init.c light.c list.c \
    matrix.c memory.c misc.c msghandling.c msghandling.h \
    opinfo.h oscontext.c oscontext.h select.c specbuf.c \
    specbuf.h texture.c tinygl.h tinyglu.h vertex.c \
    zbuffer.c zbuffer.h zdither.c zfeatures.h zgl.h zline.c \
    zline.h zmath.c zmath.h ztriangle.c ztriangle.h ztriangle_two.h

#end lib_target


#define BUILD_DIRECTORY $[HAVE_SDL]
//#define BUILDING_DLL BUILDING_TINYDISPLAY

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES sdl

#begin lib_target
  #define TARGET tinydisplay
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage

  #define SOURCES \
    config_tinydisplay.cxx config_tinydisplay.h \
    tinyGeomMunger.I tinyGeomMunger.cxx tinyGeomMunger.h \
    tinySDLGraphicsPipe.I tinySDLGraphicsPipe.cxx tinySDLGraphicsPipe.h \
    tinySDLGraphicsWindow.h tinySDLGraphicsWindow.I tinySDLGraphicsWindow.cxx \
    tinyGraphicsStateGuardian.h tinyGraphicsStateGuardian.I \
    tinyGraphicsStateGuardian.cxx \
    tinyTextureContext.I tinyTextureContext.cxx tinyTextureContext.h \
    tinyXGraphicsPipe.I tinyXGraphicsPipe.cxx tinyXGraphicsPipe.h \
    tinyXGraphicsWindow.h tinyXGraphicsWindow.I tinyXGraphicsWindow.cxx \
    clip.c error.c \
    image_util.c init.c light.c \
    memory.c msghandling.c msghandling.h \
    specbuf.c \
    texture.c vertex.c \
    zbuffer.c zbuffer.h zdither.c zfeatures.h zgl.h zline.c \
    zline.h zmath.c zmath.h ztriangle.c ztriangle.h ztriangle_two.h

#end lib_target


#define BUILD_DIRECTORY $[HAVE_TINYDISPLAY]
#define BUILDING_DLL BUILDING_TINYDISPLAY

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define USE_PACKAGES sdl

#begin lib_target
  #define TARGET tinydisplay
  #define LOCAL_LIBS \
    gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage windisplay

  #define SOURCES \
    config_tinydisplay.cxx config_tinydisplay.h \
    tinyGeomMunger.I tinyGeomMunger.cxx tinyGeomMunger.h \
    tinySDLGraphicsPipe.I tinySDLGraphicsPipe.cxx tinySDLGraphicsPipe.h \
    tinySDLGraphicsWindow.h tinySDLGraphicsWindow.I tinySDLGraphicsWindow.cxx \
    tinyGraphicsStateGuardian.h tinyGraphicsStateGuardian.I \
    tinyGraphicsStateGuardian.cxx \
    tinyTextureContext.I tinyTextureContext.cxx tinyTextureContext.h \
    tinyWinGraphicsPipe.I tinyWinGraphicsPipe.cxx tinyWinGraphicsPipe.h \
    tinyWinGraphicsWindow.h tinyWinGraphicsWindow.I tinyWinGraphicsWindow.cxx \
    tinyXGraphicsPipe.I tinyXGraphicsPipe.cxx tinyXGraphicsPipe.h \
    tinyXGraphicsWindow.h tinyXGraphicsWindow.I tinyXGraphicsWindow.cxx \
    clip.cxx error.cxx \
    image_util.cxx init.cxx light.cxx \
    memory.cxx msghandling.cxx msghandling.h \
    specbuf.cxx \
    texture.cxx vertex.cxx \
    zbuffer.cxx zbuffer.h zdither.cxx zfeatures.h zgl.h zline.cxx \
    zline.h zmath.cxx zmath.h ztriangle.cxx ztriangle.h ztriangle_two.h \
    ztriangle_code.h ztriangle_table.h 

#end lib_target


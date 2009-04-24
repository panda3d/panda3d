#define BUILD_DIRECTORY $[HAVE_TINYDISPLAY]
#define BUILDING_DLL BUILDING_TINYDISPLAY

#define OSX_SYS_FRAMEWORKS $[if $[not $[BUILD_IPHONE]],ApplicationServices Carbon CoreServices Cocoa ]
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define USE_PACKAGES sdl
#if $[UNIX_PLATFORM]
  #define USE_PACKAGES $[USE_PACKAGES] x11
#endif

#begin lib_target
  #define TARGET tinydisplay
  #define LOCAL_LIBS \
    gsgbase gobj display \
    putil linmath mathutil pnmimage windisplay

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    config_tinydisplay.h \
    tinyGeomMunger.I tinyGeomMunger.h \
    tinySDLGraphicsPipe.I tinySDLGraphicsPipe.h \
    tinySDLGraphicsWindow.h tinySDLGraphicsWindow.I \
    tinyGraphicsBuffer.h tinyGraphicsBuffer.I \
    tinyGraphicsStateGuardian.h tinyGraphicsStateGuardian.I \
    tinyTextureContext.I tinyTextureContext.h \
    tinyWinGraphicsPipe.I tinyWinGraphicsPipe.h \
    tinyWinGraphicsWindow.h tinyWinGraphicsWindow.I \
    tinyXGraphicsPipe.I tinyXGraphicsPipe.h \
    tinyXGraphicsWindow.h tinyXGraphicsWindow.I \
    tinyOffscreenGraphicsPipe.I tinyOffscreenGraphicsPipe.h \
    tinyOsxGraphicsPipe.I tinyOsxGraphicsPipe.h \
    tinyOsxGraphicsWindow.h tinyOsxGraphicsWindow.I \
    $[if $[IS_OSX],tinyOsxGraphicsWindow.mm,] \
    msghandling.h \
    zbuffer.h zfeatures.h zgl.h \
    zline.h zmath.h \
    ztriangle_1.cxx ztriangle_2.cxx \
    ztriangle_3.cxx ztriangle_4.cxx \
    ztriangle.h ztriangle_two.h \
    ztriangle_code_1.h ztriangle_code_2.h \
    ztriangle_code_3.h ztriangle_code_4.h \
    ztriangle_table.h ztriangle_table.cxx \
    store_pixel.h store_pixel_code.h store_pixel_table.h

  #define INCLUDED_SOURCES \
    clip.cxx \
    config_tinydisplay.cxx \
    error.cxx \
    image_util.cxx \
    init.cxx \
    td_light.cxx \
    memory.cxx \
    msghandling.cxx \
    specbuf.cxx \
    store_pixel.cxx \
    td_texture.cxx \
    tinyGeomMunger.cxx \
    tinyGraphicsBuffer.cxx \
    tinyGraphicsStateGuardian.cxx \
    tinyOffscreenGraphicsPipe.cxx \
    tinyOsxGraphicsPipe.cxx \
    tinySDLGraphicsPipe.cxx \
    tinySDLGraphicsWindow.cxx \
    tinyTextureContext.cxx \
    tinyWinGraphicsPipe.cxx \
    tinyWinGraphicsWindow.cxx \
    tinyXGraphicsPipe.cxx \
    tinyXGraphicsWindow.cxx \
    vertex.cxx \
    zbuffer.cxx \
    zdither.cxx \
    zline.cxx \
    zmath.cxx

#end lib_target


#define BUILD_DIRECTORY $[HAVE_X11]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m
#define USE_PACKAGES x11 xf86dga xrandr xcursor

#begin lib_target
  #define TARGET p3x11display
  #define LOCAL_LIBS p3display

  #define SOURCES \
    config_x11display.cxx config_x11display.h \
    x11GraphicsPipe.I x11GraphicsPipe.cxx x11GraphicsPipe.h \
    x11GraphicsWindow.h x11GraphicsWindow.I x11GraphicsWindow.cxx

  #define INSTALL_HEADERS \
    x11GraphicsPipe.I x11GraphicsPipe.h \
    x11GraphicsWindow.I x11GraphicsWindow.h

#end lib_target


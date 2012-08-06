// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAGL
#define BUILD_DIRECTORY $[HAVE_GL]

#define COMPONENT_LIBS \
    p3glgsg p3x11display p3glxdisplay  \
    p3wgldisplay p3osxdisplay p3cocoadisplay

#define LOCAL_LIBS p3gsgbase p3display p3express
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin metalib_target
  #define TARGET pandagl
  #define SOURCES pandagl.cxx pandagl.h
  #define INSTALL_HEADERS pandagl.h
  #define WIN_SYS_LIBS opengl32.lib winmm.lib kernel32.lib oldnames.lib user32.lib gdi32.lib
#end metalib_target

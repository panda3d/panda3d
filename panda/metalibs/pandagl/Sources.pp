// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAGL
#define BUILD_DIRECTORY $[HAVE_GL]

#if $[eq $[LINK_IN_GL],]
  // We don't have any components if we're linking the GL library
  // directly into Panda.
  #define COMPONENT_LIBS \
      glgsg glxdisplay  \
      wgldisplay
#endif

#define LOCAL_LIBS gsgbase display express
#define OTHER_LIBS dtoolconfig dtool

#begin metalib_target
  #define TARGET pandagl
  #define SOURCES pandagl.cxx pandagl.h
  #define INSTALL_HEADERS pandagl.h
  #define WIN_SYS_LIBS opengl32.lib glu32.lib winmm.lib kernel32.lib oldnames.lib user32.lib gdi32.lib
#end metalib_target

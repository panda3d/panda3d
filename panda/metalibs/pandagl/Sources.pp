#define DIRECTORY_IF_GL yes

// DIR_TYPE "metalib" indicates we are building a shared library that
// consists mostly of references to other shared libraries.  Under
// Windows, this directly produces a DLL (as opposed to the regular
// src libraries, which don't produce anything but a pile of OBJ files
// under Windows).

#define DIR_TYPE metalib
#define BUILDING_DLL BUILDING_PANDAGL

#define COMPONENT_LIBS \
    glgsg glxdisplay wgldisplay glutdisplay \
    sgidisplay sgiglxdisplay sgiglutdisplay

#define LOCAL_LIBS gsgbase display express
#define OTHER_LIBS dtoolconfig dtool

#begin metalib_target
  #define TARGET pandagl

  #define SOURCES pandagl.cxx
#end metalib_target

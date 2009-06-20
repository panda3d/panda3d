// This directory is still experimental.  Define HAVE_P3D_PLUGIN in
// your Config.pp to build it.
#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[HAVE_NPAPI]]

#define USE_PACKAGES npapi

#begin lib_target
  // By Mozilla convention, on Windows at least, the generated DLL
  // filename must begin with "np", not "libnp".
  #define TARGET nppanda3d
  #define LIB_PREFIX

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    nppanda3d_common.h \
    nppanda3d_startup.h \
    ppInstance.h ppInstance.I

  #define INCLUDED_SOURCES \
    nppanda3d_startup.cxx \
    ppInstance.cxx
 
  #define WIN_RESOURCE_FILE nppanda3d.rc
  #define LINKER_DEF_FILE nppanda3d.def

  #define INSTALL_HEADERS

#end lib_target

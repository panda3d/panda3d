// This directory builds the code for the NPAPI (Mozilla) plugin, part
// of the Panda3D browser plugin system.  Most Panda3D developers will
// have no need to build this, unless you are developing the plugin
// system itself.  Define HAVE_P3D_PLUGIN in your Config.pp to build
// this directory.

#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[HAVE_NPAPI]]

#define _MT $[if $[P3D_PLUGIN_MT],_mt]
#define USE_PACKAGES npapi gtk

#begin lib_target
  // By Mozilla convention, on Windows at least, the generated DLL
  // filename must begin with "np", not "libnp".  (Actually, this is
  // probably no longer true on recent versions of Mozilla.  But why
  // take chances?)
  #define TARGET nppanda3d
  #define LIB_PREFIX

  #define LOCAL_LIBS plugin_common$[_MT]
  #define LINK_FORCE_STATIC_RELEASE_C_RUNTIME $[P3D_PLUGIN_MT]
  #define OTHER_LIBS p3tinyxml$[_MT]

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    nppanda3d_common.h \
    ppBrowserObject.h ppBrowserObject.I \
    ppDownloadRequest.h ppDownloadRequest.I \
    ppInstance.h ppInstance.I \
    ppPandaObject.h ppPandaObject.I \
    ppToplevelObject.h ppToplevelObject.I \
    startup.h

  #define INCLUDED_SOURCES \
    ppBrowserObject.cxx \
    ppDownloadRequest.cxx \
    ppInstance.cxx \
    ppPandaObject.cxx \
    ppToplevelObject.cxx \
    startup.cxx
 
  // Windows-specific options.
  #if $[WINDOWS_PLATFORM]
    #define WIN_RESOURCE_FILE nppanda3d.rc
    #define LINKER_DEF_FILE nppanda3d.def
    #define WIN_SYS_LIBS user32.lib shell32.lib ole32.lib
  #endif

  // Mac-specific options.
  #if $[OSX_PLATFORM]
    #define LINK_AS_BUNDLE 1
    #define BUNDLE_EXT
  #endif

  #define INSTALL_HEADERS

#end lib_target


#include $[THISDIRPREFIX]nppanda3d.rc.pp

// This directory is still experimental.  Define HAVE_P3D_PLUGIN in
// your Config.pp to build it.
#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[HAVE_TINYXML],$[HAVE_OPENSSL],$[HAVE_ZLIB]]

#begin lib_target
  #define USE_PACKAGES tinyxml openssl zlib
  #define TARGET p3d_plugin
  #define LIB_PREFIX

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h \
    p3d_lock.h p3d_plugin.h \
    p3d_plugin_config.h \
    p3d_plugin_common.h \
    p3dDownload.h p3dDownload.I \
    p3dFileDownload.h p3dFileDownload.I \
    p3dFileParams.h p3dFileParams.I \
    p3dInstance.h p3dInstance.I \
    p3dInstanceManager.h p3dInstanceManager.I \
    p3dMultifileReader.h p3dMultifileReader.I \
    p3dPackage.h p3dPackage.I \
    p3dProgressWindow.h p3dProgressWindow.I \
    p3dSession.h p3dSession.I \
    p3dWindowParams.h p3dWindowParams.I \
    p3dWinProgressWindow.h p3dWinProgressWindow.I

  #define INCLUDED_SOURCES \
    p3d_plugin.cxx \
    p3dDownload.cxx \
    p3dFileDownload.cxx \
    p3dFileParams.cxx \
    p3dInstance.cxx \
    p3dInstanceManager.cxx \
    p3dMultifileReader.cxx \
    p3dPackage.cxx \
    p3dProgressWindow.cxx \
    p3dSession.cxx \
    p3dWindowParams.cxx \
    p3dWinProgressWindow.cxx

  #define INSTALL_HEADERS \
    p3d_plugin.h

  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib comctl32.lib

#end lib_target

#begin bin_target
  #define BUILD_TARGET $[HAVE_PYTHON]
  #define USE_PACKAGES tinyxml python
  #define TARGET p3dpython

  #define OTHER_LIBS \
    dtoolutil:c dtoolbase:c dtool:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    express:c pandaexpress:m \
    prc:c pstatclient:c pandabase:c linmath:c putil:c \
    pipeline:c event:c nativenet:c panda:m

  #define SOURCES \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h \
    p3d_lock.h p3d_plugin.h \
    p3d_plugin_config.h \
    p3dCInstance.cxx \
    p3dCInstance.h p3dCInstance.I \
    p3dPythonRun.cxx p3dPythonRun.h p3dPythonRun.I

#end bin_target

#begin static_lib_target
  #define TARGET plugin_common

  #define SOURCES \
     load_plugin.cxx load_plugin.h

#end static_lib_target

#include $[THISDIRPREFIX]p3d_plugin_config.h.pp

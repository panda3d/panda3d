// This directory builds the code for the ActiveX (Internet Explorer)
// plugin, part of the Panda3D browser plugin system.  Most Panda3D
// developers will have no need to build this, unless you are
// developing the plugin system itself.  Define HAVE_P3D_PLUGIN in
// your Config.pp to build this directory.

#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[HAVE_TINYXML],$[WINDOWS_PLATFORM],$[HAVE_ACTIVEX]]

#define USE_PACKAGES tinyxml

#begin lib_target
  #define TARGET p3dactivex
  #define LIB_PREFIX
  #define DYNAMIC_LIB_EXT .ocx

  #define LOCAL_LIBS plugin_common

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    P3DActiveX.h P3DActiveXCtrl.h P3DActiveXPropPage.h \
    PPBrowserObject.h PPDownloadCallback.h PPDownloadRequest.h \
    PPInstance.h PPInterface.h PPLogger.h PPPandaObject.h \
    resource.h P3DActiveX.idl

  #define INCLUDED_SOURCES \
    P3DActiveX.cpp P3DActiveXCtrl.cpp P3DActiveXPropPage.cpp \
    PPBrowserObject.cpp PPDownloadCallback.cpp PPDownloadRequest.cpp \
    PPInstance.cpp PPInterface.cpp PPLogger.cpp PPPandaObject.cpp

  #define EXTRA_CDEFS _USRDLL _WINDLL _AFXDLL _MBCS
  #define WIN_RESOURCE_FILE P3DActiveX.rc
  #define LINKER_DEF_FILE P3DActiveX.def

  #define INSTALL_HEADERS

#end lib_target

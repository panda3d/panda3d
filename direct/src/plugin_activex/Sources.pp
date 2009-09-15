// This directory is still experimental.  Define HAVE_P3D_PLUGIN in
// your Config.pp to build it.
#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[WINDOWS_PLATFORM]]

#define USE_PACKAGES tinyxml

#begin lib_target
  #define TARGET p3dactivex
  #define LIB_PREFIX
  #define DYNAMIC_LIB_EXT .ocx

  #define LOCAL_LIBS plugin_common

//  #define COMBINED_SOURCES \
//    $[TARGET]_composite1.cxx

  #define SOURCES \
    P3DActiveX.h P3DActiveXCtrl.h P3DActiveXPropPage.h P3DActiveXidl.h \
    PPBrowserObject.h PPDownloadCallback.h PPDownloadRequest.h \
    PPInstance.h PPInterface.h PPLogger.h PPPandaObject.h \
    resource.h stdafx.h

  #define SOURCES $[SOURCES] \
    P3DActiveX.idl

  #define SOURCES $[SOURCES] \
    P3DActiveX.cpp P3DActiveXCtrl.cpp P3DActiveXPropPage.cpp \
    PPBrowserObject.cpp PPDownloadCallback.cpp PPDownloadRequest.cpp \
    PPInstance.cpp PPInterface.cpp PPLogger.cpp PPPandaObject.cpp \
    stdafx.cpp

  #define EXTRA_CDEFS _USRDLL _WINDLL _AFXDLL _MBCS
  #define WIN_RESOURCE_FILE P3DActiveX.rc
  #define LINKER_DEF_FILE P3DActiveX.def

  #define INSTALL_HEADERS

#end lib_target

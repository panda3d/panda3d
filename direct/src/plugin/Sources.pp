// This directory contains the code for the "Core API" part of the
// Panda3D browser plugin system, which is not built unless you have
// defined HAVE_P3D_PLUGIN in your Config.pp.  Most Panda3D developers
// will have no need to build this, unless you are developing the
// plugin system itself.

// This directory also contains the code for p3dpython.exe, which is
// part of the Panda3D rtdist build.  It's not strictly part of the
// "Core API"; it is packaged as part of each downloadable version of
// Panda3D.  It is only built if you have defined either
// PANDA_PACKAGE_HOST_URL or HAVE_P3D_RTDIST in your Config.pp, which
// indicates an intention to build a downloadable version of Panda3D.
// Developers who are preparing a custom Panda3D package for download
// by the plugin will need to build this.

// If P3D_PLUGIN_MT is defined, then (on Windows) /MT is used to
// compile the core API and the NPAPI and ActiveX plugins, instead of
// /MD.  This links the plugin with the static C runtime library,
// instead of the dynamic runtime library, which is much better for
// distributing the plugin with the XPI and CAB interfaces.  This
// requires that special /MT versions of OpenSSL and zlib are available.

#define _MT $[if $[P3D_PLUGIN_MT],_mt]

#define COREAPI_SOURCES \
    fileSpec.cxx fileSpec.h fileSpec.I \
    find_root_dir.cxx find_root_dir.h \
    $[if $[IS_OSX],find_root_dir_assist.mm] \
    get_tinyxml.h \
    binaryXml.cxx binaryXml.h \
    fhandle.h \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h handleStreamBuf.I \
    mkdir_complete.cxx mkdir_complete.h \
    parse_color.cxx parse_color.h \
    wstring_encode.cxx wstring_encode.h \
    p3d_lock.h p3d_plugin.h \
    p3d_plugin_config.h \
    p3d_plugin_common.h \
    p3dAuthSession.h p3dAuthSession.I \
    p3dBoolObject.h \
    p3dConcreteSequence.h \
    p3dConcreteStruct.h \
    p3dConditionVar.h p3dConditionVar.I \
    p3dDownload.h p3dDownload.I \
    p3dFileDownload.h p3dFileDownload.I \
    p3dFileParams.h p3dFileParams.I \
    p3dFloatObject.h \
    p3dHost.h p3dHost.I \
    p3dInstance.h p3dInstance.I \
    p3dInstanceManager.h p3dInstanceManager.I \
    p3dIntObject.h \
    p3dMainObject.h \
    p3dMultifileReader.h p3dMultifileReader.I \
    p3dNoneObject.h \
    p3dObject.h p3dObject.I \
    p3dOsxSplashWindow.h p3dOsxSplashWindow.I \
    p3dPackage.h p3dPackage.I \
    p3dPatchfileReader.h p3dPatchfileReader.I \
    p3dPatchFinder.h p3dPatchFinder.I \
    p3dPythonObject.h \
    p3dReferenceCount.h p3dReferenceCount.I \
    p3dSession.h p3dSession.I \
    p3dSplashWindow.h p3dSplashWindow.I \
    p3dStringObject.h \
    p3dTemporaryFile.h p3dTemporaryFile.I \
    p3dUndefinedObject.h \
    p3dWinSplashWindow.h p3dWinSplashWindow.I \
    p3dX11SplashWindow.h p3dX11SplashWindow.I \
    p3dWindowParams.h p3dWindowParams.I \
    plugin_get_x11.h \
    xml_helpers.h \
    run_p3dpython.h

#define COREAPI_INCLUDED_SOURCES \
    p3d_plugin.cxx \
    p3dAuthSession.cxx \
    p3dBoolObject.cxx \
    p3dConcreteSequence.cxx \
    p3dConcreteStruct.cxx \
    p3dConditionVar.cxx \
    p3dDownload.cxx \
    p3dFileDownload.cxx \
    p3dFileParams.cxx \
    p3dFloatObject.cxx \
    p3dHost.cxx \
    p3dInstance.cxx \
    p3dInstanceManager.cxx \
    p3dIntObject.cxx \
    p3dMainObject.cxx \
    p3dMultifileReader.cxx \
    p3dNoneObject.cxx \
    p3dObject.cxx \
    p3dOsxSplashWindow.cxx \
    p3dPackage.cxx \
    p3dPatchfileReader.cxx \
    p3dPatchFinder.cxx \
    p3dPythonObject.cxx \
    p3dReferenceCount.cxx \
    p3dSession.cxx \
    p3dSplashWindow.cxx \
    p3dStringObject.cxx \
    p3dTemporaryFile.cxx \
    p3dUndefinedObject.cxx \
    p3dWinSplashWindow.cxx \
    p3dX11SplashWindow.cxx \
    p3dWindowParams.cxx \
    xml_helpers.cxx

#begin lib_target

//
// p3d_plugin.dll, the main entry point to the Core API.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_OPENSSL],$[HAVE_ZLIB]]
  #define USE_PACKAGES openssl$[_MT] zlib$[_MT] x11
  #define TARGET p3d_plugin
  #define LIB_PREFIX
  #define BUILDING_DLL BUILDING_P3D_PLUGIN
  #define LINK_FORCE_STATIC_RELEASE_C_RUNTIME $[P3D_PLUGIN_MT]

  #define OTHER_LIBS \
    p3tinyxml$[_MT] $[if $[OSX_PLATFORM],p3subprocbuffer]

  #define COMBINED_SOURCES p3d_plugin_composite1.cxx
  #define SOURCES $[COREAPI_SOURCES]
  #define INCLUDED_SOURCES $[COREAPI_INCLUDED_SOURCES]

  #define INSTALL_HEADERS \
    p3d_plugin.h

  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib comctl32.lib msimg32.lib ole32.lib

#end lib_target

#begin static_lib_target

//
// libp3d_plugin_static.lib, the Core API as a static library (for p3dembed).
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_OPENSSL],$[HAVE_ZLIB]]
  #define USE_PACKAGES openssl zlib x11
  #define TARGET p3d_plugin_static
  #define BUILDING_DLL BUILDING_P3D_PLUGIN

  #define OTHER_LIBS \
    p3tinyxml $[if $[OSX_PLATFORM],p3subprocbuffer]

  #define COMBINED_SOURCES p3d_plugin_composite1.cxx
  #define SOURCES $[COREAPI_SOURCES]
  #define INCLUDED_SOURCES $[COREAPI_INCLUDED_SOURCES]

  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib comctl32.lib msimg32.lib ole32.lib

#end static_lib_target

#begin bin_target

//
// p3dcert.exe, the authorization GUI invoked when the user clicks the
// red "play" button to approve an unknown certificate.  Considered
// part of the Core API, though it is a separate download.
//
#if $[HAVE_FLTK]
  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_FLTK],$[HAVE_OPENSSL]]
  #define USE_PACKAGES fltk openssl
  #define SOURCES p3dCert.cxx p3dCert.h
#else
  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_WX],$[HAVE_OPENSSL]]
  #define USE_PACKAGES wx openssl
  #define SOURCES p3dCert_wx.cxx p3dCert_wx.h
#endif
  #define TARGET p3dcert

  #define SOURCES $[SOURCES] \
    is_pathsep.h is_pathsep.I \
    wstring_encode.cxx wstring_encode.h \
    mkdir_complete.cxx mkdir_complete.h

  #define OSX_SYS_FRAMEWORKS Carbon

#end bin_target


#define PLUGIN_COMMON_SOURCES \
    load_plugin.cxx load_plugin.h \
    fileSpec.cxx fileSpec.h fileSpec.I \
    find_root_dir.cxx find_root_dir.h \
    $[if $[IS_OSX],find_root_dir_assist.mm] \
    is_pathsep.h is_pathsep.I \
    mkdir_complete.cxx mkdir_complete.h \
    get_twirl_data.cxx get_twirl_data.h \
    parse_color.cxx parse_color.h \
    wstring_encode.cxx wstring_encode.h


#begin static_lib_target
//
// libplugin_common.lib, a repository of code shared between the core
// API and the various plugin implementations.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_OPENSSL]]
  #define TARGET plugin_common
  #define USE_PACKAGES openssl

  #define SOURCES $[PLUGIN_COMMON_SOURCES]

#end static_lib_target

#if $[P3D_PLUGIN_MT]
#begin static_lib_target
//
// libplugin_common_mt.lib, the same as above, with /MT compilation.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_OPENSSL]]
  #define TARGET plugin_common_mt
  #define USE_PACKAGES openssl_mt
  #define LINK_FORCE_STATIC_RELEASE_C_RUNTIME 1

  #define SOURCES $[PLUGIN_COMMON_SOURCES]

#end static_lib_target
#endif  // $[P3D_PLUGIN_MT]



// The remaining targets build p3dpython.exe and variants.

#begin bin_target

//
// p3dpython.exe, the primary entry point to the downloaded Panda3D
// runtime.  This executable is run in a child process by the Core API
// to invoke a particular instance of Panda.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_RTDIST],$[HAVE_PYTHON],$[HAVE_OPENSSL]]
  #define USE_PACKAGES python openssl cg
  #define TARGET p3dpython

  #define OTHER_LIBS \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3express:c pandaexpress:m p3dxml:c \
    p3pgraph:c p3pgraphnodes:c p3cull:c p3gsgbase:c p3gobj:c \
    p3mathutil:c p3downloader:c p3pnmimage:c \
    p3prc:c p3pstatclient:c p3pandabase:c p3linmath:c p3putil:c \
    p3pipeline:c p3event:c p3display:c panda:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[HAVE_NET],p3net:c] \
    p3tinyxml

  #define SOURCES \
    binaryXml.cxx binaryXml.h \
    fhandle.h \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h handleStreamBuf.I \
    p3d_lock.h p3d_plugin.h \
    p3d_plugin_config.h \
    p3dCInstance.cxx \
    p3dCInstance.h p3dCInstance.I \
    p3dPythonRun.cxx p3dPythonRun.h p3dPythonRun.I \
    run_p3dpython.h run_p3dpython.cxx

  #define SOURCES $[SOURCES] \
    p3dPythonMain.cxx

  // If you have to link with a static Python library, define it here.
  #define EXTRA_LIBS $[EXTRA_P3DPYTHON_LIBS]
  #define OSX_SYS_FRAMEWORKS Carbon

  #define WIN_SYS_LIBS user32.lib
#end bin_target

#begin bin_target

//
// p3dpythonw.exe, a special variant on p3dpython.exe required by
// Windows (and built only on a Windows platform).  This variant is
// compiled as a desktop application, as opposed to p3dpython.exe,
// which is a console application.  (Both variants are required,
// because the plugin might be invoked either from a console or from
// the desktop.)
//

  #define BUILD_TARGET $[and $[HAVE_P3D_RTDIST],$[HAVE_PYTHON],$[HAVE_OPENSSL],$[WINDOWS_PLATFORM]]
  #define USE_PACKAGES python openssl
  #define TARGET p3dpythonw
  #define EXTRA_CDEFS NON_CONSOLE

  #define OTHER_LIBS \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3express:c pandaexpress:m p3dxml:c \
    p3pgraph:c p3pgraphnodes:c p3cull:c p3gsgbase:c p3gobj:c \
    p3mathutil:c p3downloader:c p3pnmimage:c \
    p3prc:c p3pstatclient:c p3pandabase:c p3linmath:c p3putil:c \
    p3pipeline:c p3event:c p3display:c panda:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[HAVE_NET],p3net:c] \
    p3tinyxml

  #define SOURCES \
    binaryXml.cxx binaryXml.h \
    fhandle.h \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h handleStreamBuf.I \
    p3d_lock.h p3d_plugin.h \
    p3d_plugin_config.h \
    p3dCInstance.cxx \
    p3dCInstance.h p3dCInstance.I \
    p3dPythonRun.cxx p3dPythonRun.h p3dPythonRun.I \
    run_p3dpython.h run_p3dpython.cxx

  #define SOURCES $[SOURCES] \
    p3dPythonMain.cxx

  // If you have to link with a static Python library, define it here.
  #define EXTRA_LIBS $[EXTRA_P3DPYTHON_LIBS]
  #define OSX_SYS_FRAMEWORKS Carbon

  #define WIN_SYS_LIBS user32.lib
#end bin_target

#begin lib_target

//
// libp3dpython.dll, a special library used to run P3DPythonRun within
// the parent (browser) process, instead of forking a child, as a
// desparation fallback in case forking fails for some reason.
//
  #define BUILD_TARGET $[and $[HAVE_P3D_RTDIST],$[HAVE_PYTHON],$[HAVE_OPENSSL]]
  #define USE_PACKAGES python openssl cg
  #define TARGET libp3dpython
  #define LIB_PREFIX

  #define OTHER_LIBS \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3express:c pandaexpress:m p3dxml:c \
    p3pgraph:c p3pgraphnodes:c p3cull:c p3gsgbase:c p3gobj:c \
    p3mathutil:c p3downloader:c p3pnmimage:c \
    p3prc:c p3pstatclient:c p3pandabase:c p3linmath:c p3putil:c \
    p3pipeline:c p3event:c p3display:c panda:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[HAVE_NET],p3net:c] \
    p3tinyxml

  #define SOURCES \
    binaryXml.cxx binaryXml.h \
    fhandle.h \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h handleStreamBuf.I \
    p3d_lock.h p3d_plugin.h \
    p3d_plugin_config.h \
    p3dCInstance.cxx \
    p3dCInstance.h p3dCInstance.I \
    p3dPythonRun.cxx p3dPythonRun.h p3dPythonRun.I \
    run_p3dpython.h run_p3dpython.cxx

  #define WIN_SYS_LIBS user32.lib
#end lib_target


#include $[THISDIRPREFIX]p3d_plugin_config.h.pp

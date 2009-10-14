// This directory contains the code for the "Core API" part of the
// Panda3D browser plugin system, which is not built unless you have
// defined HAVE_P3D_PLUGIN in your Config.pp.  Most Panda3D developers
// will have no need to build this, unless you are developing the
// plugin system itself.

// This directory also contains the code for p3dpython.exe, which is
// part of the Panda3D plugin runtime.  It's not strictly part of the
// "Core API"; it is packaged as part of each downloadable version of
// Panda3D.  It is only built if you have defined
// PANDA_PACKAGE_HOST_URL in your Config.pp, which indicates an
// intention to build a downloadable version of Panda3D.  Developers
// who are preparing a custom Panda3D package for download by the
// plugin will need to build this.

#begin lib_target

// 
// libp3d_plugin.dll, the main entry point to the Core API.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_TINYXML],$[HAVE_OPENSSL],$[HAVE_ZLIB],$[HAVE_JPEG],$[HAVE_PNG]]
  #define USE_PACKAGES tinyxml openssl zlib jpeg png x11
  #define TARGET p3d_plugin
  #define LIB_PREFIX

  #define OTHER_LIBS $[if $[OSX_PLATFORM],subprocbuffer]

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx

  #define SOURCES \
    fileSpec.cxx fileSpec.h fileSpec.I \
    find_root_dir.cxx find_root_dir.h \
    get_tinyxml.h \
    binaryXml.cxx binaryXml.h \
    fhandle.h \
    handleStream.cxx handleStream.h handleStream.I \
    handleStreamBuf.cxx handleStreamBuf.h handleStreamBuf.I \
    mkdir_complete.cxx mkdir_complete.h \
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
    run_p3dpython.h

  #define INCLUDED_SOURCES \
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
    p3dWindowParams.cxx

  #define INSTALL_HEADERS \
    p3d_plugin.h

  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib comctl32.lib msimg32.lib ole32.lib

#end lib_target

#begin bin_target

//
// p3dcert.exe, the authorization GUI invoked when the user clicks the
// red "play" button to approve an unknown certificate.  Considered
// part of the Core API, though it is a separate download.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_WX],$[HAVE_OPENSSL]]
  #define USE_PACKAGES wx openssl
  #define TARGET p3dcert

  #define SOURCES p3dCert.cxx
  #define OSX_SYS_FRAMEWORKS Carbon

  #if $[OSX_PLATFORM]
    // Squelch objections about ___dso_handle.
    #define LFLAGS $[LFLAGS] -undefined dynamic_lookup
  #endif
#end bin_target


#begin static_lib_target

// 
// libplugin_common.lib, a repository of code shared between the core
// API and the various plugin implementations.
//

  #define BUILD_TARGET $[and $[HAVE_P3D_PLUGIN],$[HAVE_TINYXML],$[HAVE_OPENSSL]]
  #define TARGET plugin_common
  #define USE_PACKAGES tinyxml openssl

  #define SOURCES \
    load_plugin.cxx load_plugin.h \
    fileSpec.cxx fileSpec.h fileSpec.I \
    find_root_dir.cxx find_root_dir.h \
    is_pathsep.h is_pathsep.I \
    mkdir_complete.cxx mkdir_complete.h

#end static_lib_target



// The remaining targets build p3dpython.exe and variants.

#begin bin_target

//
// p3dpython.exe, the primary entry point to the downloaded Panda3D
// runtime.  This executable is run in a child process by the Core API
// to invoke a particular instance of Panda.
//

  #define BUILD_TARGET $[and $[PANDA_PACKAGE_HOST_URL],$[HAVE_TINYXML],$[HAVE_PYTHON],$[HAVE_OPENSSL]]
  #define USE_PACKAGES tinyxml python openssl
  #define TARGET p3dpython

  #define OTHER_LIBS \
    dtoolutil:c dtoolbase:c dtool:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    express:c pandaexpress:m \
    pgraph:c pgraphnodes:c cull:c gsgbase:c gobj:c \
    mathutil:c lerp:c downloader:c pnmimage:c \
    prc:c pstatclient:c pandabase:c linmath:c putil:c \
    pipeline:c event:c nativenet:c net:c display:c panda:m

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

  #if $[OSX_PLATFORM]
    // Not entirely sure why this option is required for OSX, but we
    // get objections about ___dso_handle otherwise--but only when
    // building universal binaries.
    #define LFLAGS $[LFLAGS] -undefined dynamic_lookup
  #endif

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

  #define BUILD_TARGET $[and $[PANDA_PACKAGE_HOST_URL],$[HAVE_TINYXML],$[HAVE_PYTHON],$[HAVE_OPENSSL],$[WINDOWS_PLATFORM]]
  #define USE_PACKAGES tinyxml python openssl
  #define TARGET p3dpythonw
  #define EXTRA_CDEFS NON_CONSOLE

  #define OTHER_LIBS \
    dtoolutil:c dtoolbase:c dtool:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    express:c pandaexpress:m \
    pgraph:c pgraphnodes:c cull:c gsgbase:c gobj:c \
    mathutil:c lerp:c downloader:c pnmimage:c \
    prc:c pstatclient:c pandabase:c linmath:c putil:c \
    pipeline:c event:c nativenet:c net:c display:c panda:m

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

  #if $[OSX_PLATFORM]
    // Not entirely sure why this option is required for OSX, but we
    // get objections about ___dso_handle otherwise--but only when
    // building universal binaries.
    #define LFLAGS $[LFLAGS] -undefined dynamic_lookup
  #endif

  #define WIN_SYS_LIBS user32.lib
#end bin_target

#begin lib_target

//
// libp3dpython.dll, a special library used to run P3DPythonRun within
// the parent (browser) process, instead of forking a child, as a
// desparation fallback in case forking fails for some reason.
//

  #define BUILD_TARGET $[and $[PANDA_PACKAGE_HOST_URL],$[HAVE_TINYXML],$[HAVE_PYTHON],$[HAVE_OPENSSL]]
  #define USE_PACKAGES tinyxml python openssl
  #define TARGET libp3dpython
  #define LIB_PREFIX

  #define OTHER_LIBS \
    dtoolutil:c dtoolbase:c dtool:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    express:c pandaexpress:m \
    pgraph:c pgraphnodes:c cull:c gsgbase:c gobj:c \
    mathutil:c lerp:c downloader:c pnmimage:c \
    prc:c pstatclient:c pandabase:c linmath:c putil:c \
    pipeline:c event:c nativenet:c net:c display:c panda:m

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

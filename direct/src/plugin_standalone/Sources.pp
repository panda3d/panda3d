// This directory contains the code for the panda3d.exe executable,
// the "standalone" part of the Panda3D plugin/runtime system.  Define
// HAVE_P3D_PLUGIN in your Config.pp to build it.

#define BUILD_DIRECTORY $[and $[HAVE_P3D_PLUGIN],$[HAVE_OPENSSL],$[HAVE_ZLIB]]

#begin bin_target
  #define USE_PACKAGES openssl zlib
  #define TARGET panda3d

  #define LOCAL_LIBS plugin_common

  #define OTHER_LIBS \
    p3prc:c p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3pandabase:c p3downloader:c p3express:c pandaexpress:m \
    p3pystub p3tinyxml

  #define OSX_SYS_FRAMEWORKS Foundation AppKit Carbon

  #define SOURCES \
    panda3dBase.cxx panda3dBase.h panda3dBase.I \
    panda3d.cxx panda3d.h panda3d.I \
    panda3dMain.cxx

  #define WIN_RESOURCE_FILE panda3d.rc
  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib ole32.lib

#end bin_target

#begin bin_target
  // On Windows, we also need to build panda3dw.exe, the non-console
  // version of panda3d.exe.

  #define BUILD_TARGET $[WINDOWS_PLATFORM]
  #define USE_PACKAGES openssl zlib
  #define TARGET panda3dw

  #define LOCAL_LIBS plugin_common

  #define OTHER_LIBS \
    p3prc:c p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3pandabase:c p3downloader:c p3express:c pandaexpress:m \
    p3pystub p3tinyxml

  #define OSX_SYS_FRAMEWORKS Foundation AppKit Carbon

  #define SOURCES \
    panda3dBase.cxx panda3dBase.h panda3dBase.I \
    panda3d.cxx panda3d.h panda3d.I \
    panda3dWinMain.cxx

  #define WIN_RESOURCE_FILE panda3d.rc
  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib ole32.lib

#end bin_target

#begin bin_target
  // On Mac, we'll build panda3d_mac, which is the Carbon-friendly
  // application we wrap in a bundle, for picking a p3d file from
  // Finder.

  #define BUILD_TARGET $[OSX_PLATFORM]
  #define USE_PACKAGES openssl zlib
  #define TARGET panda3d_mac

  #define LOCAL_LIBS plugin_common

  #define OTHER_LIBS \
    p3prc:c p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3pandabase:c p3downloader:c p3express:c pandaexpress:m \
    p3pystub p3tinyxml

  #define OSX_SYS_FRAMEWORKS Foundation AppKit Carbon

  #define SOURCES \
    panda3dBase.cxx panda3dBase.h panda3dBase.I \
    panda3d.cxx panda3d.h panda3d.I \
    panda3dMac.cxx panda3dMac.h panda3dMac.I

#end bin_target

#begin bin_target
  #define USE_PACKAGES openssl zlib
  #define TARGET p3dembed
  #define LOCAL_LIBS plugin_common p3d_plugin_static

  // We need to define this, even though we are not creating a DLL,
  // because we need the symbols to be "exported" so we can find them in
  // our own address space.
  #define EXTRA_CDEFS BUILDING_P3D_PLUGIN

  #define OTHER_LIBS \
    p3prc:c p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3pandabase:c p3downloader:c p3express:c pandaexpress:m \
    p3pystub p3tinyxml \
    $[if $[OSX_PLATFORM],p3subprocbuffer]

  #define OSX_SYS_FRAMEWORKS Foundation AppKit Carbon

  #define SOURCES \
    panda3dBase.cxx panda3dBase.h panda3dBase.I \
    p3dEmbed.cxx p3dEmbedMain.cxx

  #define WIN_RESOURCE_FILE panda3d.rc
  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib comctl32.lib msimg32.lib ole32.lib

#end bin_target

#begin bin_target
  // On Windows, we also need to build p3dembedw.exe, the non-console
  // version of p3dembed.exe.

  #define BUILD_TARGET $[WINDOWS_PLATFORM]
  #define USE_PACKAGES openssl zlib
  #define TARGET p3dembedw
  #define LOCAL_LIBS plugin_common p3d_plugin_static

  // We need to define this, even though we are not creating a DLL,
  // because we need the symbols to be "exported" so we can find them in
  // our own address space.
  #define EXTRA_CDEFS BUILDING_P3D_PLUGIN P3DEMBEDW

  #define OTHER_LIBS \
    p3prc:c p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3pandabase:c p3downloader:c p3express:c pandaexpress:m \
    p3pystub p3tinyxml

  #define SOURCES \
    panda3dBase.cxx panda3dBase.h panda3dBase.I \
    p3dEmbed.cxx p3dEmbedMain.cxx

  #define WIN_RESOURCE_FILE panda3d.rc
  #define WIN_SYS_LIBS user32.lib gdi32.lib shell32.lib comctl32.lib msimg32.lib ole32.lib

#end bin_target

#include $[THISDIRPREFIX]panda3d.rc.pp

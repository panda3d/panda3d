// This file is included after including all of $DTOOL/Config.pp and
// the user's personal Config.pp file.  It makes decisions necessary 
// following the user's Config settings.

#if $[and $[OSX_PLATFORM],$[BUILD_IPHONE]]
  //#define IPH_PLATFORM iPhoneSimulator
  #define IPH_PLATFORM $[BUILD_IPHONE]
  #define IPH_VERSION 2.0
  
  #if $[eq $[IPH_PLATFORM], iPhoneOS]
    #define ARCH_FLAGS -arch armv6
    #define osflags -miphoneos-version-min=2.0
  #elif $[eq $[IPH_PLATFORM], iPhoneSimulator]
    #define ARCH_FLAGS -arch i386
    #define osflags -mmacosx-version-min=10.5
  #else
    #error Inappropriate value for BUILD_IPHONE.
  #endif
  
  #define dev /Developer/Platforms/$[IPH_PLATFORM].platform/Developer
  #define env env MACOSX_DEPLOYMENT_TARGET=10.5 PATH="$[dev]/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
  #define CC $[env] $[dev]/usr/bin/gcc-4.0
  #define CXX $[env] $[dev]/usr/bin/g++-4.0
  #define OSX_CDEFS __IPHONE_OS_VERSION_MIN_REQUIRED=20000
  #define OSX_CFLAGS -isysroot $[dev]/SDKs/$[IPH_PLATFORM]$[IPH_VERSION].sdk $[osflags] -gdwarf-2

  #defer ODIR_SUFFIX -$[IPH_PLATFORM]

#endif

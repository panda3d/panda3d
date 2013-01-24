// This file is included after including all of $DTOOL/Config.pp and
// the user's personal Config.pp file.  It makes decisions necessary 
// following the user's Config settings.

#if $[and $[OSX_PLATFORM],$[BUILD_IPHONE]]
  //#define IPH_PLATFORM iPhoneSimulator
  #define IPH_PLATFORM $[BUILD_IPHONE]
  #define IPH_VERSION 2.0
  
  #if $[eq $[IPH_PLATFORM], iPhoneOS]
    #define ARCH_FLAGS -arch armv6 -mcpu=arm1176jzf-s
    #define osflags -fpascal-strings -fasm-blocks -miphoneos-version-min=2.0
    #define DEBUGFLAGS -gdwarf-2
    //#define DEBUGFLAGS
  #elif $[eq $[IPH_PLATFORM], iPhoneSimulator]
    #define ARCH_FLAGS -arch i386
    #define osflags -fpascal-strings -fasm-blocks -mmacosx-version-min=10.5
    #define DEBUGFLAGS -gdwarf-2
  #else
    #error Inappropriate value for BUILD_IPHONE.
  #endif
  
  #define dev /Developer/Platforms/$[IPH_PLATFORM].platform/Developer
  #define env env MACOSX_DEPLOYMENT_TARGET=10.5 PATH="$[dev]/usr/bin:/Developer/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin"
  #define CC $[env] $[dev]/usr/bin/gcc-4.0
  #define CXX $[env] $[dev]/usr/bin/g++-4.0
  #define OSX_CDEFS __IPHONE_OS_VERSION_MIN_REQUIRED=20000
  #define OSX_CFLAGS -isysroot $[dev]/SDKs/$[IPH_PLATFORM]$[IPH_VERSION].sdk $[osflags]

  #defer ODIR_SUFFIX -$[IPH_PLATFORM]
#endif

#if $[eq $[PLATFORM], Android]

// These are the flags also used by Android's own ndk-build.
#if $[eq $[ANDROID_ARCH],arm]
#define target_cflags\
 -fpic\
 -ffunction-sections\
 -funwind-tables\
 -fstack-protector\
 -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__\
 -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__

#elif $[eq $[ANDROID_ARCH],mips]
#define target_cflags\
 -fpic\
 -fno-strict-aliasing\
 -finline-functions\
 -ffunction-sections\
 -funwind-tables\
 -fmessage-length=0\
 -fno-inline-functions-called-once\
 -fgcse-after-reload\
 -frerun-cse-after-loop\
 -frename-registers

#elif $[eq $[ANDROID_ABI],x86]
#define target_cflags\
 -ffunction-sections\
 -funwind-tables\
 -fstack-protector
#endif

#if $[eq $[ANDROID_ABI],armeabi-v7a]
#define target_cflags $[target_cflags]\
 -march=armv7-a \
 -mfloat-abi=softfp \
 -mfpu=vfpv3-d16

#define target_ldflags $[target_ldflags]\
 -march=armv7-a \
 -Wl,--fix-cortex-a8

#elif $[eq $[ANDROID_ABI],armeabi]
#define target_cflags $[target_cflags]\
 -march=armv5te \
 -mtune=xscale \
 -msoft-float
#endif

#define ANDROID_CFLAGS $[target_cflags] $[ANDROID_CFLAGS]
#define ANDROID_LDFLAGS $[target_ldflags] $[ANDROID_LDFLAGS]

#endif

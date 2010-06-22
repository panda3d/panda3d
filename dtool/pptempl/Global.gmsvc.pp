//
// Global.gmsvc.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.gmsvc.pp.
//

#defun get_metalibs target,complete_libs
  // In Windows, we need to know the complete set of metalibs that
  // encapsulates each of the libraries we'd be linking with normally.
  // In the case where a particular library is not part of a metalib,
  // we include the library itself.

  #define actual_libs
  #foreach lib $[complete_libs]
    // Only consider libraries that we're actually building.
    #if $[all_libs $[and $[build_directory],$[build_target]],$[lib]]
      #define modmeta $[module $[TARGET],$[lib]]
      #if $[ne $[modmeta],]
        #if $[ne $[modmeta],$[target]]  // We don't link with ourselves.
          #set actual_libs $[actual_libs] $[modmeta]
        #endif
      #else
        #set actual_libs $[actual_libs] $[lib]
      #endif
    #endif
  #end lib
  #set actual_libs $[unique $[actual_libs]] $[patsubst %:m,%,$[filter %:m,$[OTHER_LIBS]]]
  $[actual_libs]
#end get_metalibs

#defer actual_local_libs $[get_metalibs $[TARGET],$[complete_local_libs]]

#defun decygwin frompat,topat,path
  #foreach file $[path]
    $[patsubstw $[frompat],$[topat],$[osfilename $[file]]]
  #end file
#end decygwin

#define install_lib_dir $[install_lib_dir]
#define install_bin_dir $[install_bin_dir]
#define install_headers_dir $[install_headers_dir]
#define install_data_dir $[install_data_dir]
#define install_igatedb_dir $[install_igatedb_dir]
#define install_config_dir $[install_config_dir]
#define install_parser_inc_dir $[install_parser_inc_dir]

// Define this if we want to make .sbr files.
#if $[USE_BROWSEINFO]
#defer BROWSEINFO_FLAG /Fr"$[osfilename $[target:%.obj=%.sbr]]"
#else
#define BROWSEINFO_FLAG
#endif

#define CFLAGS_SHARED

#include $[THISDIRPREFIX]compilerSettings.pp

#if $[TEST_INLINING]
// /W4 will make MSVC spit out if it inlined a fn or not, but also cause a lot of other spam warnings
#define WARNING_LEVEL_FLAG /W4
#define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
#endif

// do NOT try to do #defer #defer CDEFINES_OPT1 $[CDEFINES_OPT1] here!  it wont let Sources.pp define their own CDEFINES_OPT1!  they must use EXTRA_CDEFS!
#defer CDEFINES_OPT1 $[EXTRA_CDEFS]
#defer CDEFINES_OPT2 $[EXTRA_CDEFS]
#defer CDEFINES_OPT3 $[EXTRA_CDEFS]
#defer CDEFINES_OPT4 $[EXTRA_CDEFS]

#defer cdefines $[CDEFINES_OPT$[OPTIMIZE]]

//  Opt1 /GZ disables OPT flags, so make sure its OPT1 only
#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] $[COMMONFLAGS] $[DEBUGFLAGS] $[OPT1FLAGS]
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] $[COMMONFLAGS] $[DEBUGFLAGS] $[if $[no_opt],$[OPT1FLAGS],$[OPTFLAGS]]
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[if $[no_opt],$[OPT1FLAGS],$[OPTFLAGS]] $[DEBUGPDBFLAGS]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[if $[no_opt],$[OPT1FLAGS],$[OPTFLAGS] $[OPT4FLAGS]] $[DEBUGPDBFLAGS]

//#if $[FORCE_DEBUG_FLAGS]
// make them all link with non-debug msvc runtime dlls for this case
//#defer DEBUGFLAGS $[subst /MDd,,$[DEBUGFLAGS]]
//#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS] $[DEBUGFLAGS]
//#define LINKER_FLAGS $[LINKER_FLAGS] /debug
//#else
//#endif

// NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
#defer LDFLAGS_OPT1 $[LINKER_FLAGS] $[LDFLAGS_OPT1]
#defer LDFLAGS_OPT2 $[LINKER_FLAGS] $[LDFLAGS_OPT2]
#defer LDFLAGS_OPT3 $[LINKER_FLAGS] $[LDFLAGS_OPT3]
#defer LDFLAGS_OPT4 $[LINKER_FLAGS] $[LDFLAGS_OPT4]

// $[dllext] will be "_d" for debug builds, and empty for non-debug
// builds.  This is the extra bit of stuff we tack on to the end of a
// dll name.  We name the debug dll's file_d.dll, partly to be
// consistent with Python's convention, and partly for our own benefit
// to differentiate debug-built from non-debug-built dll's (since the
// distinction is so important in Windows).
#define dllext $[if $[<= $[OPTIMIZE],2],_d]

#defer interrogate_ipath $[decygwin %,-S"%",$[install_parser_inc_dir]] $[decygwin %,-I"%",$[target_ipath]]

// '#defer extra_cflags $[extra_cflags] /STUFF' will never work because extra_cflags hasnt been
// defined yet, so this just evaluates the reference to null and removes the reference and the
// the defining extra_cflags in individual sources.pp's will not picked up.  use END_FLAGS instead
#if $[eq $[USE_COMPILER], MSVC9x64]
  #defer extra_cflags /EHsc /Zm500 /DWIN64_VC /DWIN64=1 $[WARNING_LEVEL_FLAG] $[END_CFLAGS]
#else
  #defer extra_cflags /EHsc /Zm500 /DWIN32_VC /DWIN32=1 $[WARNING_LEVEL_FLAG] $[END_CFLAGS]
#endif

#if $[direct_tau]
#define tau_ipath $[TAU_ROOT]/include
#define tau_cflags /DPROFILING_ON /DTAU_STDCXXLIB /DTAU_USE_C_API
#define tau_lpath $[TAU_ROOT]/lib/VC7
#define tau_libs pytau.lib
#else  // direct_tau
#define tau_ipath
#define tau_cflags
#define tau_lpath
#define tau_libs
#endif   // direct_tau

#defer DECYGWINED_INC_PATHLIST_ARGS $[decygwin %,/I"%",$[EXTRA_INCPATH] $[ipath] $[WIN32_PLATFORMSDK_INCPATH] $[tau_ipath]]
#defer MAIN_C_COMPILE_ARGS /nologo /c $[DECYGWINED_INC_PATHLIST_ARGS] $[flags] $[extra_cflags] $[tau_cflags] "$[osfilename $[source]]"

#defer COMPILE_C $[COMPILER] /Fo"$[osfilename $[target]]" $[MAIN_C_COMPILE_ARGS]
#defer COMPILE_C++ $[COMPILE_C]

#defer STATIC_LIB_C $[LIBBER] /nologo $[sources] /OUT:"$[osfilename $[target]]"
#defer STATIC_LIB_C++ $[STATIC_LIB_C]

#if $[eq $[USE_COMPILER], MSVC9x64]
  #defer COMPILE_IDL midl /nologo /env win64 /Oicf $[DECYGWINED_INC_PATHLIST_ARGS]
#else
  #defer COMPILE_IDL midl /nologo /env win32 /Oicf $[DECYGWINED_INC_PATHLIST_ARGS]
#endif

#defer COMPILE_RC rc /R /L 0x409 $[DECYGWINED_INC_PATHLIST_ARGS]

// if we're attached, use dllbase.txt.  otherwise let OS loader resolve dll addrspace collisions
#if $[ne $[CTPROJS],]
// use predefined bases to speed dll loading and simplify debugging
#defer DLLNAMEBASE $[lib_prefix]$[TARGET]
#defer DLLBASEADDRFILENAME dllbase.txt
#defer DLLBASEARG "/BASE:@$[dtool_ver_dir]\$[DLLBASEADDRFILENAME],$[DLLNAMEBASE]"
#else
// requires dtool envvar
#define GENERATE_BUILDDATE
#endif

#defer LINKER_DEF_FILE_ARG $[if $[LINKER_DEF_FILE],/DEF:"$[LINKER_DEF_FILE]",]

#defer SHARED_LIB_C $[LINKER] /nologo /DLL $[LINKER_DEF_FILE_ARG] $[LDFLAGS_OPT$[OPTIMIZE]] $[DLLBASEARG] /OUT:"$[osfilename $[target]]" $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH] $[tau_lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] $[tau_libs] $[VER_RESOURCE]
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C $[LINKER] /nologo $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH] $[tau_lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] $[tau_libs] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

#defer MIDL_COMMAND $[COMPILE_IDL] /out $[ODIR] $[IDL_CDEFS:%=/D%] $[idl]

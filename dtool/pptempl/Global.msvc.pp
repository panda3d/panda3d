//
// Global.msvc.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.msvc.pp.
//

#define REQUIRED_PPREMAKE_VERSION 1.02

#if $[< $[PPREMAKE_VERSION],$[REQUIRED_PPREMAKE_VERSION]]
  #error You need at least ppremake version $[REQUIRED_PPREMAKE_VERSION] to use BUILD_TYPE msvc.
#endif

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

#defun decygwin frompat,topat,path
  #foreach file $[path]
    #if $[isfullpath $[file]]
      $[patsubstw $[frompat],$[topat],$[cygpath_w $[file]]]
    #else
      $[patsubstw $[frompat],$[topat],$[osfilename $[file]]]
    #endif
  #end file
#end decygwin

#define install_lib_dir $[decygwin %,%,$[install_lib_dir]]
#define install_bin_dir $[decygwin %,%,$[install_bin_dir]]
#define install_headers_dir $[decygwin %,%,$[install_headers_dir]]
#define install_data_dir $[decygwin %,%,$[install_data_dir]]
#define install_igatedb_dir $[decygwin %,%,$[install_igatedb_dir]]
#define install_config_dir $[decygwin %,%,$[install_config_dir]]
#define install_parser_inc_dir $[decygwin %,%,$[install_parser_inc_dir]]

// In the Windows command shell, we need to use double quotes instead
// of single quotes.
#defer SED ppremake -s "$[script]" <$[source] >$[target]

// Define this if we want to make .sbr files.
#if $[USE_BROWSEINFO]
#defer BROWSEINFO_FLAG /Fr"$[osfilename $[target:%.obj=%.sbr]]"
#else
#define BROWSEINFO_FLAG
#endif

// Define LINK_ALL_STATIC to generate static libs instead of DLL's.
#if $[LINK_ALL_STATIC]
  #define dlink_all_static LINK_ALL_STATIC
  #define build_dlls
  #define dlllib lib
#else
  #define dlink_all_static
  #define build_dlls yes
  #define dlllib dll
#endif

// NO_PCH is just a user override, if it is false, 
// we still only do pch on dirs that request it
#if $[eq $[NO_PCH],]
#define DO_PCH 1
#else
#define EXTRA_CDEFS NO_PCH
#endif

#define CFLAGS_SHARED

#include $[THISDIRPREFIX]compilerSettings.pp

#define WARNING_LEVEL_FLAG /W3

#if $[TEST_INLINING]
// /W4 will make MSVC spit out if it inlined a fn or not, but also cause a lot of other spam warnings
#define WARNING_LEVEL_FLAG /W4
#define EXTRA_CDEFS $[EXTRA_CDEFS] FORCE_INLINING $[EXTRA_CDEFS]
#elif $[or $[ne $[FORCE_INLINING],],$[>= $[OPTIMIZE],2]]
#define EXTRA_CDEFS $[EXTRA_CDEFS] FORCE_INLINING $[EXTRA_CDEFS]
#endif

#defer CDEFINES_OPT1 _DEBUG $[dlink_all_static] $[EXTRA_CDEFS]
#defer CDEFINES_OPT2 _DEBUG $[dlink_all_static] $[EXTRA_CDEFS]
#defer CDEFINES_OPT3 $[dlink_all_static] $[EXTRA_CDEFS]
#defer CDEFINES_OPT4 NDEBUG $[dlink_all_static] $[EXTRA_CDEFS]

#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] $[COMMONFLAGS] $[OPT1FLAGS] $[DEBUGFLAGS]
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] $[COMMONFLAGS] $[DEBUGFLAGS] $[OPTFLAGS]
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS] $[DEBUGPDBFLAGS]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS] $[DEBUGPDBFLAGS]

// NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
#defer LDFLAGS_OPT1 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRT.LIB 
#defer LDFLAGS_OPT2 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRT.LIB 
#defer LDFLAGS_OPT3 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
#defer LDFLAGS_OPT4 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF

// $[build_pdbs] will be nonempty (true) if we should expect to
// generate a .pdb file when we build a DLL or EXE.
// we now build pdb for all Opt values
#if $[eq $[USE_COMPILER], MSVC]
  #define build_pdbs yes
#else
  #define build_pdbs
#endif

// $[dllext] will be "_d" for debug builds, and empty for non-debug
// builds.  This is the extra bit of stuff we tack on to the end of a
// dll name.  We name the debug dll's file_d.dll, partly to be
// consistent with Python's convention, and partly for our own benefit
// to differentiate debug-built from non-debug-built dll's (since the
// distinction is so important in Windows).
#define dllext $[if $[<= $[OPTIMIZE],2],_d]

#defer interrogate_ipath $[decygwin %,-I"%",$[target_ipath]]
#defer interrogate_spath $[decygwin %,-S"%",$[install_parser_inc_dir]]

#defer extra_cflags /EHsc /Zm250 /DWIN32_VC /DWIN32 $[WARNING_LEVEL_FLAG] $[END_CFLAGS]

#defer MAIN_C_COMPILE_ARGS /nologo /c $[decygwin %,/I"%",$[EXTRA_INCPATH] $[ipath] $[WIN32_PLATFORMSDK_INCPATH]] $[flags] $[extra_cflags] $[source]

#defer COMPILE_C $[COMPILER] /Fo"$[osfilename $[target]]" $[MAIN_C_COMPILE_ARGS]
#defer COMPILE_C++ $[COMPILE_C]

#if $[DO_PCH]
#defer MAIN_C_COMPILE_ARGS_PCH /Fp"$[osfilename $[target_pch]]" $[MAIN_C_COMPILE_ARGS]
#defer COMPILE_C_WITH_PCH $[COMPILER] /Yu /Fo"$[osfilename $[target]]" $[MAIN_C_COMPILE_ARGS_PCH]
#defer COMPILE_CSTYLE_PCH $[COMPILER] /TC /Yc /Fo"$[osfilename $[target_obj]]" $[MAIN_C_COMPILE_ARGS_PCH]
#defer COMPILE_CXXSTYLE_PCH $[COMPILER] /TP /Yc /Fo"$[osfilename $[target_obj]]" $[MAIN_C_COMPILE_ARGS_PCH]
#endif

#defer STATIC_LIB_C $[LIBBER] /nologo $[sources] /OUT:"$[osfilename $[target]]" 
#defer STATIC_LIB_C++ $[STATIC_LIB_C]

#if $[GENERATE_BUILDDATE]
#defer ver_resource "$[directory]\ver.res"
#else
#define ver_resource
#endif

// use predefined bases to speed dll loading and simplify debugging
#defer DLLNAMEBASE lib$[TARGET]$[dllext]
#defer DLLBASEADDRFILENAME dllbase.txt
#defer DLLBASEARG "/BASE:@$[dtool_ver_dir]\$[DLLBASEADDRFILENAME],$[DLLNAMEBASE]"

#defer SHARED_LIB_C $[LINKER] /nologo /dll $[LDFLAGS_OPT$[OPTIMIZE]] $[DLLBASEARG] $[sources] $[ver_resource] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C $[LINKER] /nologo $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

#if $[ne $[LINK_ALL_STATIC],]
  #defer SHARED_LIB_C $[STATIC_LIB_C]
  #defer SHARED_LIB_C++ $[STATIC_LIB_C++]
  #defer ODIR_SHARED $[ODIR_STATIC]
#endif


//
// Global.gmsvc.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.gmsvc.pp.
//

#define REQUIRED_PPREMAKE_VERSION 1.02

#if $[< $[PPREMAKE_VERSION],$[REQUIRED_PPREMAKE_VERSION]]
  #error You need at least ppremake version $[REQUIRED_PPREMAKE_VERSION] to use BUILD_TYPE gmsvc.
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

#if $[eq $[NO_PCH],]
#define DO_PCH 1
#else
#define EXTRA_CDEFS NO_PCH $[EXTRA_CDEFS]
#endif

#define CFLAGS_SHARED

// Define LINK_ALL_STATIC to generate static libs instead of DLL's.
#if $[ne $[LINK_ALL_STATIC],]
  #define dlink_all_static LINK_ALL_STATIC
  #define build_dlls
  #define build_libs yes  
  #define dlllib lib
#else
  #define dlink_all_static
  #define build_dlls yes
  #define build_libs  
  #define dlllib dll
#endif

#include $[THISDIRPREFIX]compilerSettings.pp

// multi-proc PCH /Z7 workaround conflicts with our need to have pdb at Opt3/4 with no increase in file size,
// so commenting this out and removing all pch for now

//#if $[and $[DO_PCH],$[>= $[NUMBER_OF_PROCESSORS],2]]
// multi-processor PCH cannot use .pdb debug fmt because
// .pdb file name must be the same for obj and pch header obj
// and currently every cxx generates its own separate pdb
// to avoid write file conflict in multi-proc build
// multi-proc case
// single-processor case will act like nmake, conditionally renaming .pdb each file in Template.gmsvc.pp
// Note: need to do /Z7 for /Zi subst to support precomp pch headers on multi-proc build
//#defer DEBUGFLAGS $[patsubst /Fd%,,$[subst /Zi,/Z7, $[DEBUGFLAGS]]]
//#define NO_PDB 1
//#else
//
// on multi-proc, since /Z7 opt required by precomp hdrs on multi-proc puts debug info into dlls,
// dont want to force debug flag by default since it expands dll size
//#define FORCE_DEBUG_FLAGS 1
//#endif

#if $[TEST_INLINING]
// /W4 will make MSVC spit out if it inlined a fn or not, but also cause a lot of other spam warnings
#define WARNING_LEVEL_FLAG /W4
#define EXTRA_CDEFS FORCE_INLINING $[EXTRA_CDEFS]
#endif

#defer CDEFINES_OPT1 _DEBUG $[dlink_all_static] $[EXTRA_CDEFS]
#defer CDEFINES_OPT2 _DEBUG $[dlink_all_static] $[EXTRA_CDEFS]
#defer CDEFINES_OPT3 $[dlink_all_static] $[EXTRA_CDEFS]
#defer CDEFINES_OPT4 NDEBUG $[dlink_all_static] $[EXTRA_CDEFS]

//  Opt1 /GZ disables OPT flags, so make sure its OPT1 only
#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] $[COMMONFLAGS] $[DEBUGFLAGS] $[OPT1FLAGS] 
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] $[COMMONFLAGS] $[DEBUGFLAGS] $[OPTFLAGS] 
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS] $[DEBUGPDBFLAGS]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS] $[DEBUGPDBFLAGS]

//#if $[FORCE_DEBUG_FLAGS]
// make them all link with non-debug msvc runtime dlls for this case
//#defer DEBUGFLAGS $[subst /MDd,,$[DEBUGFLAGS]]
//#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS] $[DEBUGFLAGS]
//#define LINKER_FLAGS $[LINKER_FLAGS] /debug
//#else
//#endif

// NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
#defer LDFLAGS_OPT1 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRT.LIB 
#defer LDFLAGS_OPT2 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRT.LIB 
#defer LDFLAGS_OPT3 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF
#defer LDFLAGS_OPT4 $[LINKER_FLAGS] /NODEFAULTLIB:MSVCRTD.LIB /OPT:REF

// $[build_pdbs] will be nonempty (true) if we should expect to
// generate a .pdb file when we build a DLL or EXE.
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

// use predefined bases to speed dll loading and simplify debugging
#defer DLLNAMEBASE lib$[TARGET]$[dllext]
#defer DLLBASEADDRFILENAME dllbase.txt
#defer DLLBASEARG "/BASE:@$[dtool_ver_dir]\$[DLLBASEADDRFILENAME],$[DLLNAMEBASE]"

//#defer ver_resource $[directory]\ver.res
//#defer SHARED_LIB_C link /nologo /dll /VERBOSE:LIB $[LDFLAGS_OPT$[OPTIMIZE]] /OUT:"$[osfilename $[target]]" $[sources] $[decygwin %,/LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] 
#defer SHARED_LIB_C $[LINKER] /nologo /dll  $[LDFLAGS_OPT$[OPTIMIZE]] $[DLLBASEARG] /OUT:"$[osfilename $[target]]" $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] 
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C $[LINKER] /nologo $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

#if $[ne $[LINK_ALL_STATIC],]
  #defer SHARED_LIB_C $[STATIC_LIB_C]
  #defer SHARED_LIB_C++ $[STATIC_LIB_C++]
  #defer ODIR_SHARED $[ODIR_STATIC]
#endif

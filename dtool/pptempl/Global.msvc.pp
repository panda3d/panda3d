//
// Global.msvc.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.msvc.pp.
//

#if $[< $[PPREMAKE_VERSION],0.55]
  #error You need at least ppremake version 0.56 to use BUILD_TYPE msvc.
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
#defer CFLAGS_SHARED

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

#defer OPTFLAGS /O2 /Ob1 /Ogity /G6

#defer CDEFINES_OPT1 _DEBUG $[dlink_all_static]
#defer CDEFINES_OPT2 _DEBUG $[dlink_all_static]
#defer CDEFINES_OPT3 $[dlink_all_static]
#defer CDEFINES_OPT4 NDEBUG $[dlink_all_static]

#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] /MDd /Gi- /GZ /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] /MDd /Gi- /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] /MD /Gi-
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] /MD /Gi-

#defer COMPILER cl
#defer LINKER link

// Define USE_BOUNDSCHECKER for BoundsChecker instrumentaion:
#if $[USE_BOUNDSCHECKER]
  #defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] /MDd /GZ /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
  #defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
  #defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] /MD
  #defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] /MD
  
  #defer OPTFLAGS /O2 /Ogity /G6
  #defer COMPILER nmcl
  #defer LINKER nmlink
#endif

// NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
#defer LDFLAGS_OPT1 /debug /incremental:no /NODEFAULTLIB:MSVCRT.LIB /WARN:3
#defer LDFLAGS_OPT2 /debug /incremental:no /NODEFAULTLIB:MSVCRT.LIB /WARN:3
#defer LDFLAGS_OPT3 /fixed:no /incremental:no /NODEFAULTLIB:MSVCRTD.LIB /WARN:3 /OPT:REF
#defer LDFLAGS_OPT4 /fixed:no /incremental:no /NODEFAULTLIB:MSVCRTD.LIB /WARN:3 /OPT:REF

// $[dllext] will be "_d" for debug builds, and empty for non-debug
// builds.  This is the extra bit of stuff we tack on to the end of a
// dll name.  We name the debug dll's file_d.dll, partly to be
// consistent with Python's convention, and partly for our own benefit
// to differentiate debug-built from non-debug-built dll's (since the
// distinction is so important in Windows).
#define dllext $[if $[<= $[OPTIMIZE],2],_d]

#defer interrogate_ipath $[decygwin %,-I"%",$[target_ipath]]
#defer interrogate_spath $[decygwin %,-S"%",$[install_parser_inc_dir]]

#if $[TEST_INLINING]
#define WARNING_LEVEL_FLAG /W4 /DTEST_INLINING
#else
#define WARNING_LEVEL_FLAG /W3
#endif

#defer extra_cflags /EHsc /Zm250 /DWIN32_VC /DWIN32 $[WARNING_LEVEL_FLAG]

#defer COMPILE_C $[COMPILER] /nologo /c /Fo"$[osfilename $[target]]" $[decygwin %,/I"%",$[ipath]] $[flags] $[extra_cflags] $[source]
#defer COMPILE_C++ $[COMPILE_C]

#defer STATIC_LIB_C lib /nologo $[sources] /OUT:"$[osfilename $[target]]" 
#defer STATIC_LIB_C++ $[STATIC_LIB_C]

#defer ver_resource $[directory]\ver.res

#defer SHARED_LIB_C $[LINKER] /nologo /dll $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] "$[ver_resource]" $[decygwin %,/LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C $[LINKER] /nologo $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

#if $[LINK_ALL_STATIC]
  #defer SHARED_LIB_C $[STATIC_LIB_C]
  #defer SHARED_LIB_C++ $[STATIC_LIB_C++]
  #defer ODIR_SHARED $[ODIR_STATIC]
#endif


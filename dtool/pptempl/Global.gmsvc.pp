//
// Global.gmsvc.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.gmsvc.pp.
//

#if $[< $[PPREMAKE_VERSION],0.58]
  #error You need at least ppremake version 0.58 to use BUILD_TYPE gmsvc.
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

#if $[eq $[USE_COMPILER], MSVC]
  #define COMPILER cl
  #define LINKER link
  #define LIBBER lib
  #define COMMONFLAGS /Gi-
  #define OPTFLAGS /O2 /Ob1 /G6
  #defer DEBUGFLAGS /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
  #define RELEASEFLAGS /MD
  
// in case we have mixed intel/msvc build
  #define EXTRA_LIBPATH /ia32/lib
  #define EXTRA_INCPATH /ia32/include    
#elif $[eq $[USE_COMPILER], BOUNDS]
  #define COMPILER nmcl
  #define LINKER nmlink
  #define LIBBER lib
  #define COMMONFLAGS
  #define OPTFLAGS /O2 /Ogity /G6
  #defer DEBUGFLAGS /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
  #define RELEASEFLAGS /MD
  #define EXTRA_LIBPATH
  #define EXTRA_INCPATH
#elif $[eq $[USE_COMPILER], INTEL]
  #define COMPILER icl
  #define LINKER xilink
  #define LIBBER xilib
  #define COMMONFLAGS /Gi-
//  #define OPTFLAGS /O3 /G6 /Qvc6 /Qwd985 /Qipo /QaxW /Qvec_report1 
  #define OPTFLAGS /O3 /G6 /Qvc6 /Qwd985
  #define DEBUGFLAGS /MDd /Zi $[BROWSEINFO_FLAG]
  #define RELEASEFLAGS /MD
  // We assume the Intel compiler installation dir is mounted as /ia32.
  #define EXTRA_LIBPATH /ia32/lib
  #define EXTRA_INCPATH /ia32/include  
#else
  #error Invalid value specified for USE_COMPILER.
#endif

#if $[PREPROCESSOR_OUTPUT]
#define COMMONFLAGS $[COMMONFLAGS] /E 
#endif 

#if $[GEN_ASSEMBLY]
#define COMMONFLAGS $[COMMONFLAGS] /FAs 
#endif 

#defer CDEFINES_OPT1 _DEBUG $[dlink_all_static]
#defer CDEFINES_OPT2 _DEBUG $[dlink_all_static]
#defer CDEFINES_OPT3 $[dlink_all_static]
#defer CDEFINES_OPT4 NDEBUG $[dlink_all_static]

#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] $[COMMONFLAGS] /GZ $[DEBUGFLAGS]
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] $[COMMONFLAGS] $[DEBUGFLAGS] $[OPTFLAGS]
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] $[COMMONFLAGS] $[RELEASEFLAGS] $[OPTFLAGS]

#if $[ENABLE_PROFILING]
// note according to docs, this should force /PDB:none /DEBUGTYPE:cv, so no pdb file is generated for debug??  (doesnt seem to be true)
#define PROFILE_FLAG /PROFILE
#else
#define PROFILE_FLAG 
#endif

// NODEFAULTLIB ensures static libs linked in will connect to the correct msvcrt, so no debug/release mixing occurs
#defer LDFLAGS_OPT1 /debug /incremental:no /NODEFAULTLIB:MSVCRT.LIB /WARN:3 $[PROFILE_FLAG]
#defer LDFLAGS_OPT2 /debug /incremental:no /NODEFAULTLIB:MSVCRT.LIB /WARN:3 $[PROFILE_FLAG] 
#defer LDFLAGS_OPT3 /fixed:no /incremental:no /NODEFAULTLIB:MSVCRTD.LIB /WARN:3 $[PROFILE_FLAG] /OPT:REF
#defer LDFLAGS_OPT4 /fixed:no /incremental:no /NODEFAULTLIB:MSVCRTD.LIB /WARN:3 $[PROFILE_FLAG] /OPT:REF

// $[build_pdbs] will be nonempty (true) if we should expect to
// generate a .pdb file when we build a DLL or EXE.
#if $[and $[eq $[USE_COMPILER], MSVC],$[<= $[OPTIMIZE],2]]
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

#if $[TEST_INLINING]
#define WARNING_LEVEL_FLAG /W4 /DTEST_INLINING
#else
#define WARNING_LEVEL_FLAG /W3
#endif

#defer extra_cflags /EHsc /Zm250 /DWIN32_VC /DWIN32 $[WARNING_LEVEL_FLAG]

#defer COMPILE_C $[COMPILER] /nologo /c /Fo"$[osfilename $[target]]" $[decygwin %,/I"%",$[EXTRA_INCPATH] $[ipath]] $[flags] $[extra_cflags] $[source]
#defer COMPILE_C++ $[COMPILE_C]

#defer STATIC_LIB_C $[LIBBER] /nologo $[sources] /OUT:"$[osfilename $[target]]" 
#defer STATIC_LIB_C++ $[STATIC_LIB_C]

//#defer ver_resource $[directory]\ver.res
//#defer SHARED_LIB_C link /nologo /dll /VERBOSE:LIB $[LDFLAGS_OPT$[OPTIMIZE]] /OUT:"$[osfilename $[target]]" $[sources] $[decygwin %,/LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] 
#defer SHARED_LIB_C $[LINKER] /nologo /dll  $[LDFLAGS_OPT$[OPTIMIZE]] /OUT:"$[osfilename $[target]]" $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] 
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C $[LINKER] /nologo $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath] $[EXTRA_LIBPATH]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

#if $[ne $[LINK_ALL_STATIC],]
  #defer SHARED_LIB_C $[STATIC_LIB_C]
  #defer SHARED_LIB_C++ $[STATIC_LIB_C++]
  #defer ODIR_SHARED $[ODIR_STATIC]
#endif

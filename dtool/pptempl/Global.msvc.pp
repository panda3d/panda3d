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

#defer OPTFLAGS /O2 /Ob1 /Ogity /G6

#defer CDEFINES_OPT1 _DEBUG
#defer CDEFINES_OPT2 _DEBUG
#defer CDEFINES_OPT3
#defer CDEFINES_OPT4 NDEBUG

#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] /MDd /Gi- /GZ /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] /MDd /Gi- /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] /MD /Gi-
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] /MD /Gi-

#defer LDFLAGS_OPT1 /debug /incremental:no
#defer LDFLAGS_OPT2 /debug /incremental:no
#defer LDFLAGS_OPT3 /fixed:no
#defer LDFLAGS_OPT4 /fixed:no

// $[dllext] will be "_d" for debug builds, and empty for non-debug
// builds.  This is the extra bit of stuff we tack on to the end of a
// dll name.  We name the debug dll's file_d.dll, partly to be
// consistent with Python's convention, and partly for our own benefit
// to differentiate debug-built from non-debug-built dll's (since the
// distinction is so important in Windows).
#define dllext $[if $[<= $[OPTIMIZE],2],_d]

#defer interrogate_ipath $[decygwin %,-I"%",$[target_ipath]]
#defer interrogate_spath $[decygwin %,-S"%",$[install_parser_inc_dir]]

#defer extra_cflags /W3 /EHsc /Zm250 /DWIN32_VC /DWIN32

#defer COMPILE_C cl /nologo /c /Fo"$[osfilename $[target]]" $[decygwin %,/I"%",$[ipath]] $[flags] $[extra_cflags] $[source]
#defer COMPILE_C++ $[COMPILE_C]

#defer STATIC_LIB_C lib /nologo $[sources] /OUT:"$[osfilename $[target]]" 
#defer STATIC_LIB_C++ $[STATIC_LIB_C]

#defer SHARED_LIB_C link /nologo /dll $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C link /nologo $[LDFLAGS_OPT$[OPTIMIZE]] $[sources] $[decygwin %,/LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

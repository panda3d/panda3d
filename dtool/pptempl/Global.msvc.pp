//
// Global.msvc.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.msvc.pp.
//

#if $[< $[PPREMAKE_VERSION],0.51]
  #error You need at least ppremake version 0.51 to use BUILD_TYPE msvc.
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

#defun decygwin frompat,topath,path
  #foreach file $[path]
    #if $[isfullpath $[file]]
      $[patsubst $[frompat],$[topath],$[cygpath_w $[file]]]
    #else
      $[patsubst $[frompat],$[topath],$[osfilename $[file]]]
    #endif
  #end file
#end decygwin

// Define this if we want to make .sbr files.
#defer BROWSEINFO_FLAG /Fr"$[osfilename $[target:%.obj=%.sbr]]"
#defer CFLAGS_SHARED

#defer OPTFLAGS /O2 /Ob1 /Ogity /G6

#defer CDEFINES_OPT1 _DEBUG
#defer CDEFINES_OPT2 _DEBUG
#defer CDEFINES_OPT3
#defer CDEFINES_OPT4 NDEBUG

#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=/D%] /MDd /GZ /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=/D%] /MDd /Zi $[BROWSEINFO_FLAG] /Fd"$[osfilename $[target:%.obj=%.pdb]]"
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=/D%] /MD /Gi-
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=/D%] /MD /Gi-

#defer LDFLAGS_OPT1 /debug /incremental:no
#defer LDFLAGS_OPT2 /debug /incremental:no
#defer LDFLAGS_OPT3 /fixed:no
#defer LDFLAGS_OPT4 /fixed:no

#defer interrogate_ipath $[decygwin %,-I"%",$[target_ipath]]
#defer interrogate_spath $[decygwin %,-S"%",$[install_parser_inc_dir]]

#defer extra_cflags /nologo /W3 /EHsc /Zm250 /D_WINDOWS /DWIN32 /D_WINDLL /DSTRICT /DPENV_WIN32 /DWIN32_VC
#defer extra_so_ldflags /DLL /NOLOGO
#defer extra_bin_ldflags /NOLOGO

#defer COMPILE_C cl /c /Fo"$[osfilename $[target]]" $[decygwin %,/I"%",$[ipath]] $[flags] $[extra_cflags] $[source]
#defer COMPILE_C++ $[COMPILE_C]

#defer SHARED_LIB_C link $[LDFLAGS_OPT$[OPTIMIZE]] $[extra_so_ldflags] $[sources] $[decygwin %,-LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer SHARED_LIB_C++ $[SHARED_LIB_C]

#defer LINK_BIN_C link $[LDFLAGS_OPT$[OPTIMIZE]] $[extra_bin_ldflags] $[sources] $[decygwin %,-LIBPATH:"%",$[lpath]] $[patsubst %.lib,%.lib,%,lib%.lib,$[libs]] /OUT:"$[osfilename $[target]]"
#defer LINK_BIN_C++ $[LINK_BIN_C]

//
// Global.stopgap.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.stopgap.pp.
//

// Define some various compile flags, derived from the variables set
// in Config.pp.
#if $[HAVE_PYTHON]
  // We want to let the PYTHON_INCLUDE directory include wildcard characters.
  #define python_ipath $[patsubst %,-I%,$[isdir $[PYTHON_IPATH]]]
  #define python_lpath $[patsubst %,-L%,$[isdir $[PYTHON_LPATH]]]
#endif

#if $[HAVE_NSPR]
  // We want to let the NSPR directories include wildcard characters.
  #define nspr_ipath $[patsubst %,-I%,$[isdir $[NSPR_IPATH]]]
  #define nspr_lpath $[patsubst %,-L%,$[isdir $[NSPR_LPATH]]]
  #define nspr_libs $[NSPR_LIBS]
#endif

#if $[HAVE_ZLIB]
  #define zlib_ipath $[ZLIB_IPATH:%=-I%]
  #define zlib_lpath $[ZLIB_LPATH:%=-L%]
  #define zlib_libs $[ZLIB_LIBS]
#endif

#if $[HAVE_SOXST]
  #define soxst_ipath $[SOXST_IPATH:%=-I%]
  #define soxst_lpath $[SOXST_LPATH:%=-L%]
  #define soxst_libs $[SOXST_LIBS]
#endif

#if $[HAVE_GL]
  #define gl_ipath $[GL_IPATH:%=-I%]
  #define gl_lpath $[GL_LPATH:%=-L%]
  #define gl_libs $[GL_LIBS]
#endif

#if $[HAVE_DX]
  #define dx_ipath $[DX_IPATH:%=-I%]
  #define dx_lpath $[DX_LPATH:%=-L%]
  #define dx_libs $[DX_LIBS]
#endif

#if $[HAVE_MIKMOD]
  #define mikmod_ipath $[MIKMOD_IPATH:%=-I%]
  #define mikmod_cflags $[MIKMOD_CFLAGS]
  #define mikmod_lpath $[MIKMOD_LPATH:%=-L%]
  #define mikmod_libs $[MIKMOD_LIBS]
#endif

#if $[HAVE_GTKMM]
  #define gtkmm_ipath $[GTKMM_IPATH:%=-I%]
  #define gtkmm_cflags $[GTKMM_CFLAGS]
  #define gtkmm_lpath $[GTKMM_LPATH:%=-L%]
  #define gtkmm_libs $[GTKMM_LIBS]
#endif

#if $[and $[HAVE_MAYA],$[MAYA_LOCATION]]
  #define maya_ipath -I$[MAYA_LOCATION]/include
  #define maya_lpath -L$[MAYA_LOCATION]/lib
  #define maya_ld $[MAYA_LOCATION]/bin/mayald
#endif

#if $[HAVE_NET]
  #define net_ipath $[NET_IPATH:%=-I%]
  #define net_lpath $[NET_LPATH:%=-L%]
  #define net_libs $[NET_LIBS]
#endif

#if $[HAVE_AUDIO]
  #define audio_ipath $[AUDIO_IPATH:%=-I%]
  #define audio_lpath $[AUDIO_LPATH:%=-L%]
  #define audio_libs $[AUDIO_LIBS]
#endif


// This variable, when evaluated in the scope of a particular directory,
// will indicate true (i.e. nonempty) when the directory is truly built,
// or false (empty) when the directory is not to be built.
#defer build_directory \
 $[and \
     $[or $[not $[DIRECTORY_IF_GL]],$[HAVE_GL]], \
     $[or $[not $[DIRECTORY_IF_DX]],$[HAVE_DX]], \
     $[or $[not $[DIRECTORY_IF_GLX]],$[HAVE_GLX]], \
     $[or $[not $[DIRECTORY_IF_GLUT]],$[HAVE_GLUT]], \
     $[or $[not $[DIRECTORY_IF_WGL]],$[HAVE_WGL]], \
     $[or $[not $[DIRECTORY_IF_RIB]],$[HAVE_RIB]], \
     $[or $[not $[DIRECTORY_IF_PS2]],$[HAVE_PS2]], \
     $[or $[not $[DIRECTORY_IF_SGIGL]],$[HAVE_SGIGL]], \
     $[or $[not $[DIRECTORY_IF_VRPN]],$[HAVE_VRPN]], \
     $[or $[not $[DIRECTORY_IF_NET]],$[HAVE_NET]], \
     $[or $[not $[DIRECTORY_IF_AUDIO]],$[HAVE_AUDIO]], \
     $[or $[not $[DIRECTORY_IF_GTKMM]],$[HAVE_GTKMM]], \
     $[or $[not $[DIRECTORY_IF_MAYA]],$[HAVE_MAYA]], \
      1 ]

// This variable is true if we are building on some flavor of Unix.
#define unix_platform $[ne $[PLATFORM],Win32]

// This variable is true if we are building on some flavor of Windows.
#define windows_platform $[eq $[PLATFORM],Win32]


// This subroutine will set up the sources variable to reflect the
// complete set of sources for this target, and also set the
// alt_cflags, alt_libs, etc. as appropriate according to how the
// various USE_* flags are set for the current target.
#defsub get_sources
  #define sources $[SOURCES]
  #if $[ne $[HAVE_ZLIB],]
    #set sources $[sources] $[IF_ZLIB_SOURCES]
  #endif
  #if $[ne $[HAVE_PYTHON],]
    #set sources $[sources] $[IF_PYTHON_SOURCES]
  #endif
  
  #define alt_cflags $[nspr_cflags] $[mikmod_cflags] $[python_cflags]
  #define alt_ipath $[nspr_ipath] $[mikmod_ipath] $[python_ipath]
  #define alt_lpath $[nspr_lpath] $[mikmod_lpath] $[python_lpath]
  #define alt_libs $[nspr_libs] $[mikmod_libs]
  #define alt_ld

  // If any of a metalib's constituent libraries require interrogate,
  // then so does the metalib itself.  To look this up, we need this map
  // variable.
  #map components TARGET(*/lib_target */noinst_lib_target)
  
  #if $[ne $[USE_ZLIB] $[components $[USE_ZLIB],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[zlib_cflags]
    #set alt_ipath $[alt_ipath] $[zlib_ipath]
    #set alt_lpath $[alt_lpath] $[zlib_lpath]
    #set alt_libs $[alt_libs] $[zlib_libs]
  #endif
  #if $[ne $[USE_GL] $[components $[USE_GL],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[gl_cflags]
    #set alt_ipath $[alt_ipath] $[gl_ipath]
    #set alt_lpath $[alt_lpath] $[gl_lpath]
    #set alt_libs $[alt_libs] $[gl_libs]
  #endif
  #if $[ne $[USE_DX] $[components $[USE_DX],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[dx_cflags]
    #set alt_ipath $[alt_ipath] $[dx_ipath]
    #set alt_lpath $[alt_lpath] $[dx_lpath]
    #set alt_libs $[alt_libs] $[dx_libs]
  #endif
  #if $[ne $[USE_SOXST] $[components $[USE_SOXST],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[soxst_cflags]
    #set alt_ipath $[alt_ipath] $[soxst_ipath]
    #set alt_lpath $[alt_lpath] $[soxst_lpath]
    #set alt_libs $[alt_libs] $[soxst_libs]
  #endif
  #if $[ne $[USE_NET] $[components $[USE_NET],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[net_cflags]
    #set alt_ipath $[alt_ipath] $[net_ipath]
    #set alt_lpath $[alt_lpath] $[net_lpath]
    #set alt_libs $[alt_libs] $[net_libs]
  #endif
  #if $[ne $[USE_AUDIO] $[components $[USE_AUDIO],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[audio_cflags]
    #set alt_ipath $[alt_ipath] $[audio_ipath]
    #set alt_lpath $[alt_lpath] $[audio_lpath]
    #set alt_libs $[alt_libs] $[audio_libs]
  #endif
  #if $[ne $[USE_GTKMM] $[components $[USE_GTKMM],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[gtkmm_cflags]
    #set alt_ipath $[alt_ipath] $[gtkmm_ipath]
    #set alt_lpath $[alt_lpath] $[gtkmm_lpath]
    #set alt_libs $[alt_libs] $[gtkmm_libs]
  #endif 
  #if $[ne $[USE_MAYA] $[components $[USE_MAYA],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[maya_cflags]
    #set alt_ipath $[alt_ipath] $[maya_ipath]
    #set alt_lpath $[alt_lpath] $[maya_lpath]
    #set alt_libs $[alt_libs] $[maya_libs]
    #set alt_ld $[maya_ld]
  #endif
  #if $[unix_platform]
    #set alt_libs $[alt_libs] $[UNIX_SYS_LIBS] $[components $[UNIX_SYS_LIBS],$[COMPONENT_LIBS]]
  #endif
#end get_sources

// This subroutine will set when_defer, when_no_defer, and when_either
// correctly to the set of libs we should link with for current
// target.
#defsub get_libs
  // For the WHEN_DEFER case, we need to know the complete set of
  // metalibs that encapsulates each of our LOCAL_LIBS.  In the case
  // where a particular library is not part of a metalib, we include the
  // library itself.
  
  // These map variables are handy to determine that.
  #map module COMPONENT_LIBS(*/metalib_target)
  #map all_libs TARGET(*/static_lib_target */lib_target */noinst_lib_target */metalib_target)
  #define when_defer
  #foreach lib $[LOCAL_LIBS]
    // Only consider libraries that we're actually building.
    #if $[all_libs $[build_directory],$[lib]]
      #define modmeta $[module $[TARGET],$[lib]]
      #if $[ne $[modmeta],]
        #set when_defer $[when_defer] $[modmeta]
      #else
        #set when_defer $[when_defer] $[lib]
      #endif
    #endif
  #end lib
  #set when_defer $[unique $[when_defer]] $[patsubst %:m,%,$[filter %:m,$[OTHER_LIBS]]]
  
  // Also filter out the libraries we don't want from when_no_defer, although
  // we don't need to translate these to metalibs.
  #define when_no_defer
  #foreach lib $[COMPONENT_LIBS] $[LOCAL_LIBS]
    #if $[all_libs $[build_directory],$[lib]]
      #set when_no_defer $[when_no_defer] $[lib]
    #endif
  #end lib
  #set when_no_defer $[unique $[when_no_defer]] $[patsubst %:c,%,$[filter %:c,$[OTHER_LIBS]]]
  
  // Finally, get the set of libraries that we want in either case.  At
  // the moment, this is just the set of libraries in OTHER_LIBS that's
  // not flagged with either a :c or a :m.
  #define when_either $[filter-out %:m %:c,$[OTHER_LIBS]]
#end get_libs


// This subroutine converts depend_libs from a list of plain library names
// to a list of the form libname.so or libname.a, according to whether the
// named libraries are static or dynamic.
#defsub convert_depend_libs
  #map static_libs TARGET(*/static_lib_target)
  #map dynamic_libs TARGET(*/lib_target */metalib_target)
  #map all_libs TARGET(*/static_lib_target */lib_target */metalib_target)
  #define new_depend_libs
  #foreach lib $[depend_libs]
    // Make sure the library is something we're actually building.
    #if $[all_libs $[build_directory],$[lib]]
      #define libname $[static_libs lib$[TARGET].a,$[lib]] $[dynamic_libs lib$[TARGET].so,$[lib]]
      #if $[eq $[libname],]
  Warning: No such library $[lib], dependency of $[DIRNAME].
      #else
        #set new_depend_libs $[new_depend_libs] $[libname]
      #endif
    #endif
  #end lib
  #set depend_libs $[sort $[new_depend_libs]]
#end convert_depend_libs


// This subroutine determines the set of libraries our various targets
// depend on.  This is a complicated definition.  It is the union of
// all of our targets' dependencies, except:

// If a target is part of a metalib, it depends (a) directly on all of
// its normal library dependencies that are part of the same metalib,
// and (b) indirectly on all of the metalibs that every other library
// dependency is part of.  If a target is not part of a metalib, it is
// the same as case (b) above.
#defsub get_depend_libs
  #map module COMPONENT_LIBS(*/metalib_target)

  #define depend_libs
  #forscopes lib_target noinst_lib_target
    #define metalib $[module $[TARGET],$[TARGET]]
    #if $[ne $[metalib],]
      // This library is included on a metalib.
      #foreach depend $[LOCAL_LIBS]
        #define depend_metalib $[module $[TARGET],$[depend]]
        #if $[eq $[depend_metalib],$[metalib]]
          // Here's a dependent library in the *same* metalib.
          #set depend_libs $[depend_libs] $[depend]
        #elif $[ne $[depend_metalib],]
          // This dependent library is in a *different* metalib.
          #set depend_libs $[depend_libs] $[depend_metalib]
        #else
          // This dependent library is not in any metalib.
          #set depend_libs $[depend_libs] $[depend]
        #endif
      #end depend
    #else
      // This library is *not* included on a metalib.
      #foreach depend $[LOCAL_LIBS]
        #define depend_metalib $[module $[TARGET],$[depend]]
        #if $[ne $[depend_metalib],]
          // This dependent library is on a metalib.
          #set depend_libs $[depend_libs] $[depend_metalib]
        #else
          // This dependent library is not in any metalib.
          #set depend_libs $[depend_libs] $[depend]
        #endif
      #end depend
    #endif
  #end lib_target noinst_lib_target
  
  // These will never be part of a metalib.
  #forscopes static_lib_target bin_target noinst_bin_target metalib_target
    #foreach depend $[LOCAL_LIBS]
      #define depend_metalib $[module $[TARGET],$[depend]]
      #if $[ne $[depend_metalib],]
        // This dependent library is on a metalib.
        #set depend_libs $[depend_libs] $[depend_metalib]
      #else
        // This dependent library is not in any metalib.
        #set depend_libs $[depend_libs] $[depend]
      #endif
    #end depend
  #end static_lib_target bin_target noinst_bin_target metalib_target

  // In case we're defining any metalibs, these depend directly on
  // their components as well.
  #set depend_libs $[depend_libs] $[COMPONENT_LIBS(metalib_target)]

  // Now correct all the libraries listed in depend_libs to refer to a
  // real library name.
  #map static_libs TARGET(*/static_lib_target)
  #map dynamic_libs TARGET(*/lib_target */metalib_target)
  #map all_libs TARGET(*/static_lib_target */lib_target */metalib_target)
  #define new_depend_libs
  #foreach lib $[sort $[depend_libs]]
    // Make sure the library is something we're actually building.
    #if $[all_libs $[build_directory],$[lib]]
      #define libname $[static_libs lib$[TARGET].a,$[lib]] $[dynamic_libs lib$[TARGET].so,$[lib]]
      #if $[eq $[libname],]
  Warning: No such library $[lib], dependency of $[DIRNAME].
      #else
        #set new_depend_libs $[new_depend_libs] $[libname]
      #endif
    #endif
  #end lib
  #set depend_libs $[sort $[new_depend_libs]]

#end get_depend_libs

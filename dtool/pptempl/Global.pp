//
// global.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables that are useful to all
// different kinds of build_types.
//


// We start off by defining a number of map variables.  These are
// special variables that can be used to look up a particular named
// scope according to a key (that is, according to the value of some
// variable defined within the scope).

// A named scope is defined using the #begin name .. #end name
// sequence.  In general, we use these sequences in the various
// Sources.pp files to define the various targets to build.  Each
// named scope carries around its set of variable declarations.  The
// named scopes are associated with the dirname of the directory in
// which the Sources.pp file was found.


// The first map variable lets us look up a particular library target
// by its target name.  The syntax here indicates that we are
// declaring a map variable called "all_libs" whose key is the
// variable $[TARGET] as defined in each instance of a named scope
// called "static_lib_target," "lib_target," and so on in every
// Sources.pp file.  (The */ refers to all the Sources.pp files.  We
// could also identify a particular file by its directory name, or
// omit the slash to refer to our own Sources.pp file.)

// After defining this map variable, we can look up other variables
// that are defined for the corresponding target.  For instances,
// $[all_libs $[SOURCES],dconfig] will return the value of the SOURCES
// variable as set for the dconfig library (that is, the expression
// $[SOURCES] is evaluated within the named scope whose key is
// "dconfig"--whose variable $[TARGET] was defined to be "dconfig").
#map all_libs TARGET(*/static_lib_target */lib_target */noinst_lib_target */metalib_target)

// These allow us to determine whether a particular local library is a
// static or a dynamic library.  If the library name appears in the
// static_libs map, it is a static library (i.e. libname.a);
// otherwise, it is a dynamic library (libname.so).
#map static_libs TARGET(*/static_lib_target)
#map dynamic_libs TARGET(*/lib_target */noinst_lib_target */metalib_target)

// This lets us identify which metalib, if any, is including each
// named library.  That is, $[module $[TARGET],name] will return
// the name of the metalib that includes library name.
#map module COMPONENT_LIBS(*/metalib_target)

// This lets up look up components of a particular metalib.
#map components TARGET(*/lib_target */noinst_lib_target)

// And this lets us look up source directories by dirname.
#map dirnames DIRNAME(*/)


// Define some various compile flags, derived from the variables set
// in Config.pp.
#set INTERROGATE_PYTHON_INTERFACE $[and $[HAVE_PYTHON],$[INTERROGATE_PYTHON_INTERFACE]]
#define run_interrogate $[or $[INTERROGATE_C_INTERFACE],$[INTERROGATE_PYTHON_INTERFACE]]

#if $[HAVE_PYTHON]
  #define python_ipath $[wildcard $[PYTHON_IPATH]]
  #define python_lpath $[wildcard $[PYTHON_LPATH]]
#endif

#if $[HAVE_NSPR]
  #define nspr_ipath $[wildcard $[NSPR_IPATH]]
  #define nspr_lpath $[wildcard $[NSPR_LPATH]]
  #define nspr_libs $[NSPR_LIBS]
#endif

#if $[HAVE_ZLIB]
  #define zlib_ipath $[wildcard $[ZLIB_IPATH]]
  #define zlib_lpath $[wildcard $[ZLIB_LPATH]]
  #define zlib_libs $[ZLIB_LIBS]
#endif

#if $[HAVE_SOXST]
  #define soxst_ipath $[wildcard $[SOXST_IPATH]]
  #define soxst_lpath $[wildcard $[SOXST_LPATH]]
  #define soxst_libs $[SOXST_LIBS]
#endif

#if $[HAVE_GL]
  #define gl_ipath $[wildcard $[GL_IPATH]]
  #define gl_lpath $[wildcard $[GL_LPATH]]
  #define gl_libs $[GL_LIBS]
#endif

#if $[HAVE_DX]
  #define dx_ipath $[wildcard $[DX_IPATH]]
  #define dx_lpath $[wildcard $[DX_LPATH]]
  #define dx_libs $[DX_LIBS]
#endif

#if $[HAVE_VRPN]
  #define vrpn_ipath $[wildcard $[VRPN_IPATH]]
  #define vrpn_lpath $[wildcard $[VRPN_LPATH]]
  #define vrpn_cflags $[VRPN_CFLAGS]
  #define vrpn_libs $[VRPN_LIBS]
#endif

#if $[HAVE_MIKMOD]
  #define mikmod_ipath $[wildcard $[MIKMOD_IPATH]]
  #define mikmod_lpath $[wildcard $[MIKMOD_LPATH]]
  #define mikmod_cflags $[MIKMOD_CFLAGS]
  #define mikmod_libs $[MIKMOD_LIBS]
#endif

#if $[HAVE_GTKMM]
  #define gtkmm_ipath $[wildcard $[GTKMM_IPATH]]
  #define gtkmm_lpath $[wildcard $[GTKMM_LPATH]]
  #define gtkmm_cflags $[GTKMM_CFLAGS]
  #define gtkmm_libs $[GTKMM_LIBS]
#endif

#if $[and $[HAVE_MAYA],$[MAYA_LOCATION]]
  #define maya_ipath -I$[MAYA_LOCATION]/include
  #define maya_lpath -L$[MAYA_LOCATION]/lib
  #define maya_ld $[MAYA_LOCATION]/bin/mayald
#endif

#if $[HAVE_NET]
  #define net_ipath $[wildcard $[NET_IPATH]]
  #define net_lpath $[wildcard $[NET_LPATH]]
  #define net_libs $[NET_LIBS]
#endif

#if $[HAVE_AUDIO]
  #define audio_ipath $[wildcard $[AUDIO_IPATH]]
  #define audio_lpath $[wildcard $[AUDIO_LPATH]]
  #define audio_libs $[AUDIO_LIBS]
#endif


// This variable, when evaluated in the scope of a particular directory,
// will indicate true (i.e. nonempty) when the directory is truly built,
// or false (empty) when the directory is not to be built.
#defer build_directory \
 $[and \
     $[or $[not $[DIRECTORY_IF_PYTHON]],$[HAVE_PYTHON]], \
     $[or $[not $[DIRECTORY_IF_NSPR]],$[HAVE_NSPR]], \
     $[or $[not $[DIRECTORY_IF_ZLIB]],$[HAVE_ZLIB]], \
     $[or $[not $[DIRECTORY_IF_SOXST]],$[HAVE_SOXST]], \
     $[or $[not $[DIRECTORY_IF_GL]],$[HAVE_GL]], \
     $[or $[not $[DIRECTORY_IF_DX]],$[HAVE_DX]], \
     $[or $[not $[DIRECTORY_IF_GLX]],$[HAVE_GLX]], \
     $[or $[not $[DIRECTORY_IF_GLUT]],$[HAVE_GLUT]], \
     $[or $[not $[DIRECTORY_IF_WGL]],$[HAVE_WGL]], \
     $[or $[not $[DIRECTORY_IF_RIB]],$[HAVE_RIB]], \
     $[or $[not $[DIRECTORY_IF_PS2]],$[HAVE_PS2]], \
     $[or $[not $[DIRECTORY_IF_SGIGL]],$[HAVE_SGIGL]], \
     $[or $[not $[DIRECTORY_IF_VRPN]],$[HAVE_VRPN]], \
     $[or $[not $[DIRECTORY_IF_GTKMM]],$[HAVE_GTKMM]], \
     $[or $[not $[DIRECTORY_IF_MAYA]],$[HAVE_MAYA]], \
     $[or $[not $[DIRECTORY_IF_NET]],$[HAVE_NET]], \
     $[or $[not $[DIRECTORY_IF_AUDIO]],$[HAVE_AUDIO]], \
      1 ]

// This variable, when evaluated in the scope of a particular target,
// will indicated true when the target should be built, or false when
// the target is not to be built.
#defer build_target \
 $[and \
     $[or $[not $[TARGET_IF_PYTHON]],$[HAVE_PYTHON]], \
     $[or $[not $[TARGET_IF_NSPR]],$[HAVE_NSPR]], \
     $[or $[not $[TARGET_IF_ZLIB]],$[HAVE_ZLIB]], \
     $[or $[not $[TARGET_IF_SOXST]],$[HAVE_SOXST]], \
     $[or $[not $[TARGET_IF_GL]],$[HAVE_GL]], \
     $[or $[not $[TARGET_IF_DX]],$[HAVE_DX]], \
     $[or $[not $[TARGET_IF_GLX]],$[HAVE_GLX]], \
     $[or $[not $[TARGET_IF_GLUT]],$[HAVE_GLUT]], \
     $[or $[not $[TARGET_IF_WGL]],$[HAVE_WGL]], \
     $[or $[not $[TARGET_IF_RIB]],$[HAVE_RIB]], \
     $[or $[not $[TARGET_IF_PS2]],$[HAVE_PS2]], \
     $[or $[not $[TARGET_IF_SGIGL]],$[HAVE_SGIGL]], \
     $[or $[not $[TARGET_IF_VRPN]],$[HAVE_VRPN]], \
     $[or $[not $[TARGET_IF_GTKMM]],$[HAVE_GTKMM]], \
     $[or $[not $[TARGET_IF_MAYA]],$[HAVE_MAYA]], \
     $[or $[not $[TARGET_IF_NET]],$[HAVE_NET]], \
     $[or $[not $[TARGET_IF_AUDIO]],$[HAVE_AUDIO]], \
      1 ]

// This takes advantage of the above two variables to get the actual
// list of local libraries we are to link with, eliminating those that
// won't be built.
#defer active_local_libs \
  $[all_libs $[if $[and $[build_directory],$[build_target]],$[TARGET]],$[LOCAL_LIBS]]
#defer active_component_libs \
  $[all_libs $[if $[and $[build_directory],$[build_target]],$[TARGET]],$[COMPONENT_LIBS]]
#defer active_libs $[active_local_libs] $[active_component_libs]

// This variable is true if we are building on some flavor of Unix.
#define unix_platform $[ne $[PLATFORM],Win32]

// This variable is true if we are building on some flavor of Windows.
#define windows_platform $[eq $[PLATFORM],Win32]


// This subroutine will set up the sources variable to reflect the
// complete set of sources for this target, and also set the
// alt_cflags, alt_libs, etc. as appropriate according to how the
// various USE_* flags are set for the current target.

// This variable returns the complete set of sources for the current
// target.
#defer get_sources \
  $[SOURCES] \
  $[if $[HAVE_ZLIB],$[IF_ZLIB_SOURCES]] \
  $[if $[HAVE_PYTHON],$[IF_PYTHON_SOURCES]]

// This variable returns the set of sources that are to be
// interrogated for the current target.
#defer get_igatescan \
  $[if $[and $[run_interrogate],$[IGATESCAN]], \
     $[if $[eq $[IGATESCAN], all], \
      $[filter-out %.I %.lxx %.yxx %.N,$[get_sources]], \
      $[IGATESCAN]]]

// This variable returns the name of the interrogate database file
// that will be generated for a particular target, or empty string if
// the target is not to be interrogated.
#defer get_igatedb \
  $[if $[and $[run_interrogate],$[IGATESCAN]], \
    lib$[TARGET].in]

// This variable returns the name of the interrogate module, if the
// current metalib target should include one, or empty string if it
// should not.
#defer get_igatemscan \
  $[if $[and $[run_interrogate],$[components $[IGATESCAN],$[COMPONENT_LIBS]]], \
    $[TARGET]]
    


// This function returns the appropriate cflags for the target, based
// on the various external packages this particular target claims to
// require.
#defun get_cflags
  #define alt_cflags $[nspr_cflags] $[python_cflags]
  
  #if $[ne $[USE_ZLIB] $[components $[USE_ZLIB],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[zlib_cflags]
  #endif
  #if $[ne $[USE_GL] $[components $[USE_GL],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[gl_cflags]
  #endif
  #if $[ne $[USE_DX] $[components $[USE_DX],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[dx_cflags]
  #endif
  #if $[ne $[USE_SOXST] $[components $[USE_SOXST],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[soxst_cflags]
  #endif
  #if $[ne $[USE_NET] $[components $[USE_NET],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[net_cflags]
  #endif
  #if $[ne $[USE_AUDIO] $[components $[USE_AUDIO],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[audio_cflags]
  #endif
  #if $[ne $[USE_MIKMOD] $[components $[USE_MIKMOD],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[mikmod_cflags]
  #endif
  #if $[ne $[USE_GTKMM] $[components $[USE_GTKMM],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[gtkmm_cflags]
  #endif 
  #if $[ne $[USE_MAYA] $[components $[USE_MAYA],$[COMPONENT_LIBS]],]
    #set alt_cflags $[alt_cflags] $[maya_cflags]
  #endif

  $[alt_cflags]
#end get_cflags

// This function returns the appropriate include path for the target,
// based on the various external packages this particular target
// claims to require.  This returns a space-separated set of directory
// names only; the -I switch is not included here.
#defun get_ipath
  #define alt_ipath $[nspr_ipath] $[python_ipath]
  
  #if $[ne $[USE_ZLIB] $[components $[USE_ZLIB],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[zlib_ipath]
  #endif
  #if $[ne $[USE_GL] $[components $[USE_GL],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[gl_ipath]
  #endif
  #if $[ne $[USE_DX] $[components $[USE_DX],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[dx_ipath]
  #endif
  #if $[ne $[USE_SOXST] $[components $[USE_SOXST],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[soxst_ipath]
  #endif
  #if $[ne $[USE_NET] $[components $[USE_NET],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[net_ipath]
  #endif
  #if $[ne $[USE_AUDIO] $[components $[USE_AUDIO],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[audio_ipath]
  #endif
  #if $[ne $[USE_MIKMOD] $[components $[USE_MIKMOD],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[mikmod_ipath]
  #endif
  #if $[ne $[USE_GTKMM] $[components $[USE_GTKMM],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[gtkmm_ipath]
  #endif 
  #if $[ne $[USE_MAYA] $[components $[USE_MAYA],$[COMPONENT_LIBS]],]
    #set alt_ipath $[alt_ipath] $[maya_ipath]
  #endif

  $[alt_ipath]
#end get_ipath

// This function returns the appropriate library search path for the
// target, based on the various external packages this particular
// target claims to require.  This returns a space-separated set of
// directory names only; the -L switch is not included here.
#defun get_lpath
  #define alt_lpath $[nspr_lpath] $[python_lpath]
  
  #if $[ne $[USE_ZLIB] $[components $[USE_ZLIB],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[zlib_lpath]
  #endif
  #if $[ne $[USE_GL] $[components $[USE_GL],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[gl_lpath]
  #endif
  #if $[ne $[USE_DX] $[components $[USE_DX],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[dx_lpath]
  #endif
  #if $[ne $[USE_SOXST] $[components $[USE_SOXST],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[soxst_lpath]
  #endif
  #if $[ne $[USE_NET] $[components $[USE_NET],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[net_lpath]
  #endif
  #if $[ne $[USE_AUDIO] $[components $[USE_AUDIO],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[audio_lpath]
  #endif
  #if $[ne $[USE_MIKMOD] $[components $[USE_MIKMOD],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[mikmod_lpath]
  #endif
  #if $[ne $[USE_GTKMM] $[components $[USE_GTKMM],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[gtkmm_lpath]
  #endif 
  #if $[ne $[USE_MAYA] $[components $[USE_MAYA],$[COMPONENT_LIBS]],]
    #set alt_lpath $[alt_lpath] $[maya_lpath]
  #endif

  $[alt_lpath]
#end get_lpath

// This function returns the appropriate set of library names to link
// with for the target, based on the various external packages this
// particular target claims to require.  This returns a
// space-separated set of library names only; the -l switch is not
// included here.
#defun get_libs
  #define alt_libs $[nspr_libs] $[python_libs]
  
  #if $[ne $[USE_ZLIB] $[components $[USE_ZLIB],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[zlib_libs]
  #endif
  #if $[ne $[USE_GL] $[components $[USE_GL],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[gl_libs]
  #endif
  #if $[ne $[USE_DX] $[components $[USE_DX],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[dx_libs]
  #endif
  #if $[ne $[USE_SOXST] $[components $[USE_SOXST],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[soxst_libs]
  #endif
  #if $[ne $[USE_NET] $[components $[USE_NET],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[net_libs]
  #endif
  #if $[ne $[USE_AUDIO] $[components $[USE_AUDIO],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[audio_libs]
  #endif
  #if $[ne $[USE_MIKMOD] $[components $[USE_MIKMOD],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[mikmod_libs]
  #endif
  #if $[ne $[USE_GTKMM] $[components $[USE_GTKMM],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[gtkmm_libs]
  #endif 
  #if $[ne $[USE_MAYA] $[components $[USE_MAYA],$[COMPONENT_LIBS]],]
    #set alt_libs $[alt_libs] $[maya_libs]
  #endif
  #if $[unix_platform]
    #set alt_libs $[alt_libs] $[UNIX_SYS_LIBS] $[components $[UNIX_SYS_LIBS],$[COMPONENT_LIBS]]
  #endif

  $[alt_libs]
#end get_libs

// This function returns the appropriate value for ld for the target.
#defun get_ld
  #if $[ne $[USE_MAYA] $[components $[USE_MAYA],$[COMPONENT_LD]],]
    mayald
  #endif
#end get_ld


// Include the global definitions for this particular build_type, if
// the file is there.
#sinclude $[GLOBAL_TYPE_FILE]


//
// Global.pp
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
#map all_libs TARGET(*/static_lib_target */ss_lib_target */lib_target */noinst_lib_target */test_lib_target */metalib_target)

// This map variable allows us to look up global variables that might
// be defined in a particular Sources.pp, e.g. in the "toplevel" file.
#map dir_type DIR_TYPE(*/)

// These allow us to determine whether a particular local library is a
// static or a dynamic library.  If the library name appears in the
// static_libs map, it is a static library (i.e. libname.a);
// otherwise, it is a dynamic library (libname.so).  The target
// ss_lib_target is a special case: these libraries are dynamic where
// it's easy to make them so (e.g. on Unix platforms), and static on
// platforms where dynamic libraries aren't quite so robust (e.g. on
// Windows).
#if $[WINDOWS_PLATFORM]
  #map static_libs TARGET(*/static_lib_target */ss_lib_target)
  #map dynamic_libs TARGET(*/lib_target */noinst_lib_target */test_lib_target */metalib_target)
#else
  #map static_libs TARGET(*/static_lib_target)
  #map dynamic_libs TARGET(*/lib_target */ss_lib_target */noinst_lib_target */test_lib_target */metalib_target)
#endif

// This lets us identify which metalib, if any, is including each
// named library.  That is, $[module $[TARGET],name] will return
// the name of the metalib that includes library name.
#map module COMPONENT_LIBS(*/metalib_target)

// This lets up look up components of a particular metalib.
#map components TARGET(*/lib_target */noinst_lib_target */test_lib_target)

// And this lets us look up source directories by dirname.
#map dirnames DIRNAME(*/)

// This is used by Template.models.pp.
#if $[HAVE_SOFTIMAGE]
  #define SOFT2EGG soft -D libsoftegg soft2egg
#else
  // We used to use the old converter from pre-Panda days.  Now this
  // is no longer supported.
  //  #define SOFT2EGG soft2egg
  #define SOFT2EGG soft -D libsoftegg soft2egg
#endif

// Define some various compile flags, derived from the variables set
// in Config.pp.
#set INTERROGATE_PYTHON_INTERFACE $[and $[HAVE_PYTHON],$[INTERROGATE_PYTHON_INTERFACE]]
#define run_interrogate $[HAVE_INTERROGATE]

#define stl_ipath $[wildcard $[STL_IPATH]]
#define stl_lpath $[wildcard $[STL_LPATH]]
#define stl_cflags $[STL_CFLAGS]
#define stl_libs $[STL_LIBS]

#if $[HAVE_PYTHON]
  #define python_ipath $[wildcard $[PYTHON_IPATH]]
  #define python_lpath $[wildcard $[PYTHON_LPATH]]
  #define python_fpath $[wildcard $[PYTHON_FPATH]]
  #define python_cflags $[PYTHON_CFLAGS]
  #define python_lflags $[PYTHON_LFLAGS]
  #define python_libs $[PYTHON_LIBS]
  #define python_framework $[PYTHON_FRAMEWORK]
#endif

#if $[HAVE_NSPR]
  #define nspr_ipath $[wildcard $[NSPR_IPATH]]
  #define nspr_lpath $[wildcard $[NSPR_LPATH]]
  #define nspr_cflags $[NSPR_CFLAGS]
  #define nspr_libs $[NSPR_LIBS]
#endif

#if $[HAVE_SSL]
  #define ssl_ipath $[wildcard $[SSL_IPATH]]
  #define ssl_lpath $[wildcard $[SSL_LPATH]]
  #define ssl_cflags $[SSL_CFLAGS]
  #define ssl_libs $[SSL_LIBS]
#endif

#if $[HAVE_ZLIB]
  #define zlib_ipath $[wildcard $[ZLIB_IPATH]]
  #define zlib_lpath $[wildcard $[ZLIB_LPATH]]
  #define zlib_cflags $[ZLIB_CFLAGS]
  #define zlib_libs $[ZLIB_LIBS]
#endif

#if $[HAVE_GL]
  #define gl_ipath $[wildcard $[GL_IPATH]]
  #define gl_lpath $[wildcard $[GL_LPATH]]
  #define gl_cflags $[GL_CFLAGS]
  #define gl_libs $[GL_LIBS]
#endif

#if $[HAVE_MESA]
  #define mesa_ipath $[wildcard $[MESA_IPATH]]
  #define mesa_lpath $[wildcard $[MESA_LPATH]]
  #define mesa_cflags $[MESA_CFLAGS]
  #define mesa_libs $[MESA_LIBS]
#endif

#if $[HAVE_CHROMIUM]
  #define chromium_ipath $[wildcard $[CHROMIUM_IPATH]]
  #define chromium_lpath $[wildcard $[CHROMIUM_LPATH]]
  #define chromium_cflags $[CHROMIUM_CFLAGS]
  #define chromium_libs $[CHROMIUM_LIBS]
#endif

#if $[HAVE_GLX]
  #define glx_ipath $[wildcard $[GLX_IPATH]]
  #define glx_lpath $[wildcard $[GLX_LPATH]]
  #define glx_cflags $[GLX_CFLAGS]
  #define glx_libs $[GLX_LIBS]
#endif

#if $[HAVE_GLUT]
  #define glut_ipath $[wildcard $[GLUT_IPATH]]
  #define glut_lpath $[wildcard $[GLUT_LPATH]]
  #define glut_cflags $[GLUT_CFLAGS]
  #define glut_libs $[GLUT_LIBS]
#endif

#if $[HAVE_DX]
  #define dx_ipath $[wildcard $[DX_IPATH]]
  #define dx_lpath $[wildcard $[DX_LPATH]]
  #define dx_cflags $[DX_CFLAGS]
  #define dx_libs $[DX_LIBS]
#endif

#if $[HAVE_JPEG]
  #define jpeg_ipath $[wildcard $[JPEG_IPATH]]
  #define jpeg_lpath $[wildcard $[JPEG_LPATH]]
  #define jpeg_cflags $[JPEG_CFLAGS]
  #define jpeg_libs $[JPEG_LIBS]
#endif

#if $[HAVE_PNG]
  #define png_ipath $[wildcard $[PNG_IPATH]]
  #define png_lpath $[wildcard $[PNG_LPATH]]
  #define png_cflags $[PNG_CFLAGS]
  #define png_libs $[PNG_LIBS]
#endif

#if $[HAVE_TIFF]
  #define tiff_ipath $[wildcard $[TIFF_IPATH]]
  #define tiff_lpath $[wildcard $[TIFF_LPATH]]
  #define tiff_cflags $[TIFF_CFLAGS]
  #define tiff_libs $[TIFF_LIBS]
#endif

#if $[HAVE_FFTW]
  #define fftw_ipath $[wildcard $[FFTW_IPATH]]
  #define fftw_lpath $[wildcard $[FFTW_LPATH]]
  #define fftw_cflags $[FFTW_CFLAGS]
  #define fftw_libs $[FFTW_LIBS]
#endif

#if $[HAVE_NURBSPP]
  #define nurbspp_ipath $[wildcard $[NURBSPP_IPATH]]
  #define nurbspp_lpath $[wildcard $[NURBSPP_LPATH]]
  #define nurbspp_cflags $[NURBSPP_CFLAGS]
  #define nurbspp_libs $[NURBSPP_LIBS]
#endif

#if $[HAVE_CG]
  #define cg_ipath $[wildcard $[CG_IPATH]]
  #define cg_lpath $[wildcard $[CG_LPATH]]
  #define cg_cflags $[CG_CFLAGS]
  #define cg_libs $[CG_LIBS]
#endif

#if $[HAVE_CGGL]
  #define cggl_ipath $[wildcard $[CGGL_IPATH]]
  #define cggl_lpath $[wildcard $[CGGL_LPATH]]
  #define cggl_cflags $[CGGL_CFLAGS]
  #define cggl_libs $[CGGL_LIBS]
#endif

#if $[HAVE_VRPN]
  #define vrpn_ipath $[wildcard $[VRPN_IPATH]]
  #define vrpn_lpath $[wildcard $[VRPN_LPATH]]
  #define vrpn_cflags $[VRPN_CFLAGS]
  #define vrpn_libs $[VRPN_LIBS]
#endif

#if $[HAVE_HELIX]
  #define helix_ipath $[wildcard $[HELIX_IPATH]]
  #define helix_lpath $[wildcard $[HELIX_LPATH]]
  #define helix_cflags $[HELIX_CFLAGS]
  #define helix_libs $[HELIX_LIBS]
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

#if $[HAVE_FREETYPE]
  #define freetype_ipath $[wildcard $[FREETYPE_IPATH]]
  #define freetype_lpath $[wildcard $[FREETYPE_LPATH]]
  #define freetype_cflags $[FREETYPE_CFLAGS]
  #define freetype_libs $[FREETYPE_LIBS]
#endif

#if $[and $[HAVE_MAYA],$[MAYA_LOCATION]]
  #define maya_ipath $[MAYA_LOCATION]/include
  #define maya_lpath $[MAYA_LOCATION]/lib
  #define maya_ld $[wildcard $[MAYA_LOCATION]/bin/mayald]
  #define maya_libs $[MAYA_LIBS]
#endif

#if $[and $[HAVE_SOFTIMAGE],$[SOFTIMAGE_LOCATION]]
  #define softimage_ipath $[SOFTIMAGE_LOCATION]/h
  #define softimage_lpath $[SOFTIMAGE_LOCATION]/dso
  #define softimage_libs $[SOFTIMAGE_LIBS]
#endif

#if $[HAVE_NET]
  #define net_ipath $[wildcard $[NET_IPATH]]
  #define net_lpath $[wildcard $[NET_LPATH]]
  #define net_libs $[NET_LIBS]
#endif

#if $[HAVE_RAD_MSS]
  #define rad_mss_ipath $[wildcard $[RAD_MSS_IPATH]]
  #define rad_mss_lpath $[wildcard $[RAD_MSS_LPATH]]
  #define rad_mss_libs $[RAD_MSS_LIBS]
#endif

#if $[HAVE_FMOD]
  #define fmod_ipath $[wildcard $[FMOD_IPATH]]
  #define fmod_lpath $[wildcard $[FMOD_LPATH]]
  #define fmod_libs $[FMOD_LIBS]
#endif

#if $[HAVE_CHROMIUM]
  #define chromium_ipath $[wildcard $[CHROMIUM_IPATH]]
  #define chromium_lpath $[wildcard $[CHROMIUM_LPATH]]
  #define chromium_libs $[CHROMIUM_LIBS]
#endif

// We define these two variables true here in the global scope; a
// particular Sources.pp file can redefine these to be false to
// prevent a particular directory or target from being built in
// certain circumstances.
#define BUILD_DIRECTORY 1
#define BUILD_TARGET 1

// This variable, when evaluated in the scope of a particular directory,
// will indicate true (i.e. nonempty) when the directory is truly built,
// or false (empty) when the directory is not to be built.
#defer build_directory $[BUILD_DIRECTORY]
// It maps to a direct evaluation of the user-set variable,
// BUILD_DIRECTORY, for historical reasons.  This also allows us to
// reserve the right to extend this variable to test other conditions
// as well, should the need arise.

// This variable, when evaluated in the scope of a particular target,
// will indicated true when the target should be built, or false when
// the target is not to be built.
#defer build_target $[BUILD_TARGET]


// This takes advantage of the above two variables to get the actual
// list of local libraries we are to link with, eliminating those that
// won't be built.
#defer active_local_libs \
  $[all_libs $[if $[and $[build_directory],$[build_target]],$[TARGET]],$[LOCAL_LIBS]]
#defer active_component_libs \
  $[all_libs $[if $[and $[build_directory],$[build_target]],$[TARGET]],$[COMPONENT_LIBS]]
#defer active_libs $[active_local_libs] $[active_component_libs]


// This variable, when evaluated within a target, will either be empty
// string if the target is not to be built, or the target name if it
// is.
#defer active_target $[if $[build_target],$[TARGET]]
#defer get_combined_sources $[COMBINED_SOURCES]

// This subroutine will set up the sources variable to reflect the
// complete set of sources for this target, and also set the
// alt_cflags, alt_libs, etc. as appropriate according to how the
// various USE_* flags are set for the current target.

// This variable returns the complete set of sources for the current
// target.

#defer get_sources \
  $[SOURCES] \
  $[if $[ne $[NO_COMBINED_SOURCES],], $[INCLUDED_SOURCES], $[get_combined_sources]]

#defer included_sources $[INCLUDED_SOURCES]

// This variable returns the set of sources that are to be
// interrogated for the current target.
#defer get_igatescan \
  $[if $[and $[run_interrogate],$[IGATESCAN]], \
     $[if $[eq $[IGATESCAN], all], \
      $[filter-out %.I %.T %.lxx %.yxx %.N %_src.cxx,$[get_sources]], \
      $[IGATESCAN]]]

// This variable returns the name of the interrogate database file
// that will be generated for a particular target, or empty string if
// the target is not to be interrogated.
#defer get_igatedb \
  $[if $[and $[run_interrogate],$[IGATESCAN]], \
    $[ODIR]/lib$[TARGET]$[dllext].in]

// This variable returns the name of the interrogate code file
// that will be generated for a particular target, or empty string if
// the target is not to be interrogated.
#defer get_igateoutput \
  $[if $[and $[run_interrogate],$[IGATESCAN]], \
    $[ODIR]/lib$[TARGET]_igate.cxx]

// This variable is the set of .in files generated by all of our
// component libraries.  If it is nonempty, then we do need to
// generate a module, and $[get_igatemout] is the name of the .cxx file
// that interrogate will produce to make this module.
#defer get_igatemscan $[components $[get_igatedb:%=$[RELDIR]/%],$[active_component_libs]]
#defer get_igatemout $[if $[get_igatemscan],$[ODIR]/lib$[TARGET]_module.cxx]

// This variable returns the set of external packages used by this
// target, and by all the components shared by this target.
#defer use_packages $[sort $[USE_PACKAGES] $[components $[USE_PACKAGES],$[active_component_libs]]]

// This function returns the appropriate cflags for the target, based
// on the various external packages this particular target claims to
// require.
#defun get_cflags
  // hack to add stl,nspr,python.  should be removed
  #define alt_cflags $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_cflags] $[nspr_cflags] $[python_cflags]]

  #foreach package $[use_packages]
    #set alt_cflags $[alt_cflags] $[$[package]_cflags]
  #end package

  $[alt_cflags]
#end get_cflags

// This function returns the appropriate lflags for the target, based
// on the various external packages this particular target claims to
// require.
#defun get_lflags
  // hack to add stl,nspr,python.  should be removed
  #define alt_lflags $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_lflags] $[nspr_lflags] $[python_lflags]]

  #foreach package $[use_packages]
    #set alt_lflags $[alt_lflags] $[$[package]_lflags]
  #end package

  $[alt_lflags]
#end get_lflags

// This function returns the appropriate include path for the target,
// based on the various external packages this particular target
// claims to require.  This returns a space-separated set of directory
// names only; the -I switch is not included here.
#defun get_ipath
  // hack to add stl,nspr,python.  should be removed
  #define alt_ipath $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_ipath] $[nspr_ipath] $[python_ipath]]

  #foreach package $[use_packages]
    #set alt_ipath $[alt_ipath] $[$[package]_ipath]
  #end package

  $[alt_ipath]
#end get_ipath

// This function returns the appropriate library search path for the
// target, based on the various external packages this particular
// target claims to require.  This returns a space-separated set of
// directory names only; the -L switch is not included here.
#defun get_lpath
  #define alt_lpath $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_lpath] $[nspr_lpath] $[python_lpath]]

  #if $[WINDOWS_PLATFORM]
    #set alt_lpath $[WIN32_PLATFORMSDK_LIBPATH] $[alt_lpath]
  #endif

  #foreach package $[use_packages]
    #set alt_lpath $[alt_lpath] $[$[package]_lpath]
  #end package

  $[alt_lpath]
#end get_lpath

// This function returns the appropriate framework search path for the
// target, based on the various external frameworks this particular
// target claims to require.  This returns a space-separated set of
// directory names only; the -F switch is not included here.
#defun get_fpath
  #define alt_fpath $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_fpath] $[nspr_fpath] $[python_fpath]]

  #foreach package $[use_packages]
    #set alt_fpath $[alt_fpath] $[$[package]_fpath]
  #end package

  $[alt_fpath]
#end get_fpath

// This function returns the appropriate framework for the
// target, based on the various external frameworks this particular
// target claims to require.  This returns a space-separated set of
// framework names only; the -framework switch is not included here.
#defun get_frameworks
  #define alt_framework $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_framework] $[nspr_framework] $[python_framework]]

  #foreach package $[use_packages]
    #set alt_framework $[alt_framework] $[$[package]_framework]
  #end package

  $[alt_framework]
#end get_frameworks

// This function returns the appropriate set of library names to link
// with for the target, based on the various external packages this
// particular target claims to require.  This returns a
// space-separated set of library names only; the -l switch is not
// included here.
#defun get_libs
  #define alt_libs $[if $[IGNORE_LIB_DEFAULTS_HACK],,$[stl_libs] $[nspr_libs] $[python_libs]]

  #define alt_libs $[alt_libs] $[EXTRA_LIBS]

  #if $[WINDOWS_PLATFORM]
    #set alt_libs $[alt_libs] $[WIN_SYS_LIBS] $[components $[WIN_SYS_LIBS],$[active_libs] $[transitive_link]]
  #elif $[OSX_PLATFORM]
    #set alt_libs $[alt_libs] $[OSX_SYS_LIBS] $[components $[OSX_SYS_LIBS],$[active_libs] $[transitive_link]]
  #else
    #set alt_libs $[alt_libs] $[UNIX_SYS_LIBS] $[components $[UNIX_SYS_LIBS],$[active_libs] $[transitive_link]]
  #endif

  #foreach package $[use_packages]
    #set alt_libs $[alt_libs] $[$[package]_libs]
  #end package

  $[alt_libs]
#end get_libs

// This function returns the appropriate value for ld for the target.
#defun get_ld
  #if $[filter maya,$[use_packages]]
    $[maya_ld]
  #endif
#end get_ld

// This function determines the set of files a given source file
// depends on.  It is based on the setting of the $[filename]_sources
// variable to indicate the sources for composite files, etc.
#defun get_depends source
  #if $[$[source]_sources]
    #if $[ne $[$[source]_sources],none]
      $[$[source]_sources] $[dependencies $[$[source]_sources]]
    #endif
  #else
    $[dependencies $[source]]
  #endif
#end get_depends


// This function determines the set of libraries our various targets
// depend on.  This is a complicated definition.  It is the union of
// all of our targets' dependencies, except:

// If a target is part of a metalib, it depends (a) directly on all of
// its normal library dependencies that are part of the same metalib,
// and (b) indirectly on all of the metalibs that every other library
// dependency is part of.  If a target is not part of a metalib, it is
// the same as case (b) above.
#defun get_depend_libs
  #define depend_libs
  #forscopes lib_target noinst_lib_target test_lib_target
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
  #end lib_target noinst_lib_target test_lib_target

  // These will never be part of a metalib.
  #forscopes static_lib_target ss_lib_target bin_target noinst_bin_target metalib_target
    #foreach depend $[LOCAL_LIBS]
      #define depend_metalib $[module $[TARGET],$[depend]]
      #if $[ne $[depend_metalib],]
        // This dependent library is on a metalib.
        #if $[eq $[depend_metalib],$[TARGET]]
          #print Warning: $[TARGET] circularly depends on $[depend].
        #else
          #set depend_libs $[depend_libs] $[depend_metalib]
        #endif
      #else
        // This dependent library is not in any metalib.
        #set depend_libs $[depend_libs] $[depend]
      #endif
    #end depend
  #end static_lib_target ss_lib_target bin_target noinst_bin_target metalib_target

  // In case we're defining any metalibs, these depend directly on
  // their components as well.
  #set depend_libs $[depend_libs] $[COMPONENT_LIBS(metalib_target)]

  $[depend_libs]
#end get_depend_libs


// dtool/pptempl/Global.pp

// Define a few directories that will be useful.

#define install_dir $[$[upcase $[PACKAGE]]_INSTALL]
#if $[eq $[install_dir],]
  #error Variable $[upcase $[PACKAGE]]_INSTALL is not set!  Cannot install!
#endif

#define other_trees
#define other_trees_lib
#define other_trees_include
#foreach tree $[NEEDS_TREES]
  #define tree_install $[$[upcase $[tree]]_INSTALL]
  #if $[eq $[tree_install],]
Warning: Variable $[upcase $[tree]]_INSTALL is not set!
  #else
    #set other_trees $[other_trees] $[tree_install]
    #set other_trees_lib $[other_trees_lib] $[tree_install]/lib
    #set other_trees_include $[other_trees_include] $[tree_install]/include
  #endif
#end tree

#define install_lib_dir $[or $[INSTALL_LIB_DIR],$[install_dir]/lib]
#define other_trees_lib $[or $[INSTALL_LIB_DIR],$[other_trees_lib]]

#define install_headers_dir $[or $[INSTALL_HEADERS_DIR],$[install_dir]/include]
#define other_trees_include $[or $[INSTALL_HEADERS_DIR],$[other_trees_include]]

#define install_bin_dir $[or $[INSTALL_BIN_DIR],$[install_dir]/bin]
#define install_data_dir $[or $[INSTALL_DATA_DIR],$[install_dir]/shared]
#define install_igatedb_dir $[or $[INSTALL_IGATEDB_DIR],$[install_dir]/etc]
#define install_config_dir $[or $[INSTALL_CONFIG_DIR],$[install_dir]/etc]
#defer install_py_dir $[install_lib_dir]/$[PACKAGE]/$[DIRNAME]
#defer install_py_package_dir $[install_lib_dir]/$[PACKAGE]

#if $[ne $[DTOOL_INSTALL],]
  #define install_parser_inc_dir $[DTOOL_INSTALL]/include/parser-inc
#else
  #define install_parser_inc_dir $[install_headers_dir]/parser-inc
#endif

// Set up the correct interrogate options.

// $[dllext] is redefined in the Windows Global.platform.pp files to
// the string _d if we are building a debug tree.  This is inserted
// into the .dll and .in filenames before the extension to make a
// runtime distinction between debug and non-debug builds.  For now,
// we make a global definition to empty string, since non-Windows
// platforms will leave this empty.
#define dllext

// $[obj_prefix] defines the prefix that is prepended to the name of
// the object files.  It can be used to avoid potential collisions
// when a source file is used by multiple targets but with different
// compile options for each.
//
// $[obj_prefix] may be redefined by one of the Global.platform.pp
// files.
#defer obj_prefix $[TARGET]_

// Caution!  interrogate_ipath might be redefined in the
// Global.platform.pp file.
#defer interrogate_ipath $[target_ipath:%=-I%]
#defer interrogate_spath $[install_parser_inc_dir:%=-S%] -S/usr/include

#defer interrogate_options \
    -DCPPPARSER -D__STDC__=1 -D__cplusplus $[SYSTEM_IGATE_FLAGS] \
    $[interrogate_spath] $[interrogate_ipath] \
    $[CDEFINES_OPT$[OPTIMIZE]:%=-D%] \
    $[filter -D%,$[C++FLAGS]] \
    $[INTERROGATE_OPTIONS] \
    $[if $[INTERROGATE_PYTHON_INTERFACE],$[if $[PYTHON_NATIVE],-python-native,-python]] \
    $[if $[INTERROGATE_C_INTERFACE],-c] \
    $[if $[TRACK_IN_INTERPRETER],-track-interpreter] \
    $[if $[<= $[OPTIMIZE], 1],-spam]

#defer interrogate_module_options \
    $[if $[INTERROGATE_PYTHON_INTERFACE],$[if $[PYTHON_NATIVE],-python-native,-python]] \
    $[if $[INTERROGATE_C_INTERFACE],-c] \
    $[if $[TRACK_IN_INTERPRETER],-track-interpreter]


// The language stuff is used by model builds only.
// Set language_filters to be "%_english %_castillian %_japanese %_german" etc.
#if $[LANGUAGES]
  #define language_filters $[subst <pct>,%,$[LANGUAGES:%=<pct>_%]]
  #print Using language $[LANGUAGE]
#else
  #define language_filters
#endif
#define language_egg_filters $[language_filters:%=%.egg]
#define language_dna_filters $[language_filters:%=%.dna]

// This is used for evaluating SoftImage unpack rules in Template.models.pp.
#defer soft_scene_files $[matrix $[DATABASE]/SCENES/$[SCENE_PREFIX],$[MODEL] $[ANIMS],.1-0.dsc]

// Include the global definitions for this particular build_type, if
// the file is there.
#sinclude $[GLOBAL_TYPE_FILE]


//
// Template.nmake.pp
//
// This file defines the set of output files that will be generated to
// support a makefile build system invoking Microsoft's Visual C++
// command-line compiler, similar to Template.msvc.pp
//
// It is a clone of Template.gmsvc.pp that has been adapter do work
// with nmake and visual studio.  I didn't call it Template.msvc.pp
// so as not to confuse it with the previous Template.msvc.pp which
// attempts to create visual studio projects.
//
// Steven "Sauce" Osman, July 17, 2003 
//
// Note:
// There's one wierd behavior when running interrogate, specifically
// with its -od directive.  This is because the pathname specified
// in -od is dropped into a string in a c++ source file.  This makes
// the c++ compiler interpret all back-slashes as escape sequences.
// To fix this, the back-slashes were replaced by double backslashes.
// This doesn't seem to break interrogate's ability to open the files
// (because additional slashes don't bother the OS), and it allows
// the escape character interpretation to work.
//
// Note:
// The SED variable uses single quotes around the command.  A
// substitution was added to make them double quotes.

// Before this file is processed, the following files are read and
// processed (in order):

// The Package.pp file in the root of the current source hierarchy
//   (e.g. $PANDA/Package.pp)
// $DTOOL/Package.pp
// $DTOOL/Config.pp
// $DTOOL/Config.Platform.pp
// The user's PPREMAKE_CONFIG file.
// $DTOOL/pptempl/System.pp
// All of the Sources.pp files in the current source hierarchy
// $DTOOL/pptempl/Global.pp
// $DTOOL/pptempl/Global.nmake.pp
// $DTOOL/pptempl/Depends.pp, once for each Sources.pp filem
// Template.nmake.pp (this file), once for each Sources.pp file

#if $[ne $[CTPROJS],]
#define dtool_ver_dir_cyg $[DTOOL]/src/dtoolbase
#define dtool_ver_dir $[osfilename $[DTOOL]/src/dtoolbase]
#endif

//////////////////////////////////////////////////////////////////////
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib]]
//////////////////////////////////////////////////////////////////////
// For a source directory, build a single Makefile with rules to build
// each target.

#if $[build_directory]
  // This is the real set of lib_targets we'll be building.  On Windows,
  // we don't build the shared libraries which are included on metalibs.
  #define real_lib_targets
  #define real_lib_target_libs
  #define deferred_objs
  #forscopes lib_target
    #if $[eq $[module $[TARGET],$[TARGET]],]
      // This library is not on a metalib, so we can build it.
      #set real_lib_targets $[real_lib_targets] $[TARGET]
      #set real_lib_target_libs $[real_lib_target_libs] $[ODIR]/$[get_dllname $[TARGET]].$[dlllib]
    #else
      // This library is on a metalib, so we can't build it, but we
      // should build all the obj's that go into it.
      #set deferred_objs $[deferred_objs] \
        $[patsubst %,$[%_obj],$[compile_sources]]
    #endif
  #end lib_target

  // We need to know the various targets we'll be building.
  // $[lib_targets] will be the list of dynamic libraries,
  // $[static_lib_targets] the list of static libraries, and
  // $[bin_targets] the list of binaries.  $[test_bin_targets] is the
  // list of binaries that are to be built only when specifically asked for.

  #define lib_targets $[forscopes metalib_target noinst_lib_target test_lib_target,$[if $[build_target],$[ODIR]/$[get_dllname $[TARGET]].$[dlllib]]] $[real_lib_target_libs]
  #define static_lib_targets $[forscopes static_lib_target ss_lib_target,$[if $[build_target],$[ODIR]/$[get_dllname $[TARGET]].lib]]

  #define bin_targets \
      $[active_target(bin_target noinst_bin_target):%=$[ODIR]/%.exe] \
      $[active_target(sed_bin_target):%=$[ODIR]/%]
  #define test_bin_targets $[active_target(test_bin_target):%=$[ODIR]/%.exe]

  #defer test_lib_targets $[active_target(test_lib_target):%=$[if $[TEST_ODIR],$[TEST_ODIR],$[ODIR]]/%.$[dlllib]]

  // And these variables will define the various things we need to
  // install.
  #define install_lib $[active_target(metalib_target static_lib_target ss_lib_target)] $[real_lib_targets]
  #define install_bin $[active_target(bin_target)]
  #define install_scripts $[sort $[INSTALL_SCRIPTS(metalib_target lib_target static_lib_target ss_lib_target bin_target)] $[INSTALL_SCRIPTS]]
  #define install_headers $[sort $[INSTALL_HEADERS(metalib_target lib_target static_lib_target ss_lib_target bin_target)] $[INSTALL_HEADERS]]
  #define install_parser_inc $[sort $[INSTALL_PARSER_INC]]
  #define install_data $[sort $[INSTALL_DATA(metalib_target lib_target static_lib_target ss_lib_target bin_target)] $[INSTALL_DATA]]
  #define install_config $[sort $[INSTALL_CONFIG(metalib_target lib_target static_lib_target ss_lib_target bin_target)] $[INSTALL_CONFIG]]
  #define install_igatedb $[sort $[get_igatedb(metalib_target lib_target)]]

  // These are the various sources collected from all targets within the
  // directory.
  #define st_sources $[sort $[compile_sources(metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target)]]
  #define yxx_st_sources $[sort $[yxx_sources(metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target)]]
  #define lxx_st_sources $[sort $[lxx_sources(metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target)]]
  #define dep_sources_1  $[sort $[get_sources(metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target)]]

  // These are the source files that our dependency cache file will
  // depend on.  If it's an empty list, we won't bother writing rules to
  // freshen the cache file.
  #define dep_sources $[sort $[filter %.c %.cxx %.cpp %.yxx %.lxx %.h %.I %.T,$[dep_sources_1]]]

#endif  // $[build_directory]


// We define $[complete_local_libs] as the full set of libraries (from
// within this tree) that we must link a particular target with.  It
// is the transitive closure of our dependent libs: the libraries we
// depend on, plus the libraries *those* libraries depend on, and so on.
#defer complete_local_libs $[unique $[closure all_libs,$[active_libs]]]
#defer actual_local_libs $[get_metalibs $[TARGET],$[complete_local_libs]]

// $[static_lib_dependencies] is the set of libraries we will link
// with that happen to be static libs.  We will introduce dependency
// rules for these.  (We don't need dependency rules for dynamic libs,
// since these don't get burned in at build time.)
#defer static_lib_dependencies $[all_libs $[if $[lib_is_static],$[RELDIR:%=%/$[ODIR]/lib$[TARGET]$[dllext].lib]],$[complete_local_libs]]

// And $[complete_ipath] is the list of directories (from within this
// tree) we should add to our -I list.  It's basically just one for
// each directory named in the $[complete_local_libs], above, plus
// whatever else the user might have explicitly named in
// $[LOCAL_INCS].  LOCAL_INCS MUST be a ppremake src dir! (RELDIR only checks those)
// To add an arbitrary extra include dir, define EXTRA_IPATH in the Sources.pp

#defer complete_ipath $[all_libs $[RELDIR],$[complete_local_libs]] $[RELDIR($[LOCAL_INCS:%=%/])] $[EXTRA_IPATH]

// $[target_ipath] is the proper ipath to put on the command line,
// from the context of a particular target.

#defer target_ipath $[TOPDIR] $[sort $[complete_ipath]] $[other_trees:%=%/include] $[get_ipath]

// These are the complete set of extra flags the compiler requires.
#defer cflags $[get_cflags] $[CFLAGS] $[CFLAGS_OPT$[OPTIMIZE]]
#defer c++flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]]

// $[complete_lpath] is rather like $[complete_ipath]: the list of
// directories (from within this tree) we should add to our -L list.
#defer complete_lpath $[static_libs $[RELDIR:%=%/$[ODIR]],$[actual_local_libs]] $[dynamic_libs $[RELDIR:%=%/$[ODIR]],$[actual_local_libs]]

// $[lpath] is like $[target_ipath]: it's the list of directories we
// should add to our -L list, from the context of a particular target.
#defer lpath $[sort $[complete_lpath]] $[other_trees:%=%/lib] $[get_lpath]

// And $[libs] is the set of libraries we will link with.
#defer libs $[unique $[actual_local_libs:%=%$[dllext]] $[patsubst %:c,,%:m %,%$[dllext],$[OTHER_LIBS]] $[get_libs]]

// This is the set of files we might copy into *.prebuilt, if we have
// bison and flex (or copy from *.prebuilt if we don't have them).
#define bison_prebuilt $[patsubst %.yxx,%.cxx %.h,$[yxx_st_sources]] $[patsubst %.lxx,%.cxx,$[lxx_st_sources]]

// Rather than making a rule to generate each install directory later,
// we create the directories now.  This reduces problems from
// multiprocess builds.
#mkdir $[sort \
    $[if $[install_lib],$[install_lib_dir]] \
    $[if $[install_bin] $[install_scripts],$[install_bin_dir]] \
    $[if $[install_headers],$[install_headers_dir]] \
    $[if $[install_parser_inc],$[install_parser_inc_dir]] \
    $[if $[install_data],$[install_data_dir]] \
    $[if $[install_config],$[install_config_dir]] \
    $[if $[install_igatedb],$[install_igatedb_dir]] \
    ]

// Similarly, we need to ensure that $[ODIR] exists.  Trying to make
// the makefiles do this automatically just causes problems with
// multiprocess builds.
#mkdir $[ODIR] $[TEST_ODIR]

// Pre-compiled headers are one way to speed the compilation of many
// C++ source files that include similar headers, but it turns out a
// more effective (and more portable) way is simply to compile all the
// similar source files in one pass.

// We do this by generating a *_composite.cxx file that has an
// #include line for each of several actual source files, and then we
// compile the composite file instead of the original files.
#foreach composite_file $[composite_list]
#output $[composite_file] notouch
#format collapse
/* Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE]. */
/* ################################# DO NOT EDIT ########################### */

#foreach file $[$[composite_file]_sources]
##include "$[file]"
#end file

#end $[composite_file]
#end composite_file

// Okay, we're ready.  Start outputting the Makefile now.
#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

// The 'all' rule makes all the stuff in the directory except for the
// test_bin_targets.  It doesn't do any installation, however.
#define all_targets \
    Makefile \
    $[if $[dep_sources],$[DEPENDENCY_CACHE_FILENAME]] \
    $[sort $[lib_targets] $[static_lib_targets] $[bin_targets]] \
    $[deferred_objs]
all : $[patsubst %,$[osfilename %],$[all_targets]]

// The 'test' rule makes all the test_bin_targets.
test : $[patsubst %,$[osfilename %],$[test_bin_targets] $[test_lib_targets]]

clean : clean-igate
#if $[st_sources]
#foreach file $[patsubst %,$[osfilename $[%_obj]],$[st_sources]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif
#if $[deferred_objs]
 #foreach file $[patsubst %,$[osfilename %],$[deferred_objs]]
$[TAB] if exist $[file] del /f $[file]
 #end file
#endif
#if $[lib_targets] $[static_lib_targets] $[bin_targets] $[test_bin_targets]
#foreach file $[patsubst %,$[osfilename %],$[lib_targets] $[static_lib_targets] $[bin_targets] $[test_bin_targets]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif
$[TAB] if exist *.pyc del *.pyc
$[TAB] if exist *.pyo del *.pyo // Also scrub out old generated Python code.

// 'cleanall' is intended to undo all the effects of running ppremake
// and building.  It removes everything except the Makefile.
cleanall : clean
#if $[st_sources]
$[TAB] if exist $[osfilename $[ODIR]\*.*] del /f/s/q $[osfilename $[ODIR]\*.*]
#endif
#if $[yxx_st_sources] $[lxx_st_sources]
#foreach file $[patsubst %,$[osfilename %],$[patsubst %.yxx,%.cxx %.h,$[yxx_st_sources]] $[patsubst %.lxx,%.cxx,$[lxx_st_sources]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif
#if $[ne $[DEPENDENCY_CACHE_FILENAME],]
$[TAB] if exist $[osfilename $[DEPENDENCY_CACHE_FILENAME]] del /f $[osfilename $[DEPENDENCY_CACHE_FILENAME]]
#endif
#if $[composite_list]
#foreach file $[patsubst %,$[osfilename %],$[composite_list]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

clean-igate :
#forscopes metalib_target lib_target ss_lib_target
  #define igatedb $[get_igatedb]
  #define igateoutput $[get_igateoutput]
  #define igatemscan $[get_igatemscan]
  #define igatemout $[get_igatemout]
  #if $[igatedb]
$[TAB] if exist $[osfilename $[igatedb]] del /f $[osfilename $[igatedb]]
  #endif
  #if $[igateoutput]
#foreach file $[patsubst %,$[osfilename %],$[igateoutput] $[$[igateoutput]_obj]]
$[TAB] if exist $[file] del /f $[file]
#end file
  #endif
  #if $[igatemout]
#foreach file $[patsubst %,$[osfilename %],$[igatemout] $[$[igatemout]_obj]]
$[TAB] if exist $[file] del /f $[file]
#end file
  #endif
#end metalib_target lib_target ss_lib_target

// Now, 'install' and 'uninstall'.  These simply copy files into the
// install directory (or remove them).  The 'install' rule also makes
// the directories if necessary.
#define installed_files \
     $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
     $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
     $[INSTALL_PARSER_INC:%=$[install_parser_inc_dir]/%] \
     $[INSTALL_DATA:%=$[install_data_dir]/%] \
     $[INSTALL_CONFIG:%=$[install_config_dir]/%]

#define installed_igate_files \
     $[get_igatedb(metalib_target lib_target ss_lib_target):$[ODIR]/%=$[install_igatedb_dir]/%]

#define install_targets \
     $[active_target(metalib_target lib_target static_lib_target ss_lib_target):%=install-lib%] \
     $[active_target(bin_target sed_bin_target):%=install-%] \
     $[installed_files]

install : $[patsubst %,$[osfilename %],all $[install_targets]]

install-igate : $[patsubst %,$[osfilename %],$[sort $[installed_igate_files]]]

uninstall : $[active_target(metalib_target lib_target static_lib_target ss_lib_target):%=uninstall-lib%] $[active_target(bin_target):%=uninstall-%]
#if $[installed_files]
#foreach file $[patsubst %,$[osfilename %],$[sort $[installed_files]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

uninstall-igate :
#if $[installed_igate_files]
#foreach file $[patsubst %,$[osfilename %],$[sort $[installed_igate_files]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

#if $[HAVE_BISON]
prebuild-bison : $[patsubst %,$[osfilename %],$[patsubst %,%.prebuilt,$[bison_prebuilt]]]
clean-prebuild-bison :
#if $[bison_prebuilt]
#foreach file $[patsubst %,$[osfilename %],$[sort $[patsubst %,%.prebuilt,$[bison_prebuilt]]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif
#endif

// Now it's time to start generating the rules to make our actual
// targets.

igate : $[patsubst %,$[osfilename %],$[get_igatedb(metalib_target lib_target ss_lib_target)]]


/////////////////////////////////////////////////////////////////////
// First, the dynamic libraries.  Each lib_target and metalib_target
// is a dynamic library.
/////////////////////////////////////////////////////////////////////

#forscopes metalib_target lib_target

// In Windows, we don't actually build all the libraries.  In
// particular, we don't build any libraries that are listed on a
// metalib.  Is this one such a library?
#define build_it $[eq $[module $[TARGET],$[TARGET]],]

// We might need to define a BUILDING_ symbol for win32.  We use the
// BUILDING_DLL variable name, defined typically in the metalib, for
// this; but in some cases, where the library isn't part of a metalib,
// we define BUILDING_DLL directly for the target.
#define building_var $[or $[BUILDING_DLL],$[module $[BUILDING_DLL],$[TARGET]]]

// $[igatescan] is the set of C++ headers and source files that we
// need to scan for interrogate.  $[igateoutput] is the name of the
// generated .cxx file that interrogate will produce (and which we
// should compile into the library).  $[igatedb] is the name of the
// generated .in file that interrogate will produce (and which should
// be installed into the /etc directory).
#define igatescan $[get_igatescan]
#define igateoutput $[get_igateoutput]
#define igatedb $[get_igatedb]

// If this is a metalib, it may have a number of components that
// include interrogated interfaces.  If so, we need to generate a
// 'module' file within this library.  This is mainly necessary for
// Python; it contains a table of all of the interrogated functions,
// so we can load the library as a Python module and have access to
// the interrogated functions.

// $[igatemscan] is the set of .in files generated by all of our
// component libraries.  If it is nonempty, then we do need to
// generate a module, and $[igatemout] is the name of the .cxx file
// that interrogate will produce to make this module.
#define igatemscan $[get_igatemscan]
#define igatemout $[get_igatemout]

#if $[build_it]
  // Now output the rule to actually link the library from all of its
  // various .obj files.

  #define sources \
   $[patsubst %,$[%_obj],$[compile_sources]] \
   $[components $[patsubst %,$[RELDIR]/$[%_obj],$[compile_sources]],$[active_component_libs]]

  #define varname $[subst -,_,lib$[TARGET]_so]
$[varname] = $[patsubst %,$[osfilename %],$[sources]]
  #define target $[ODIR]/$[get_dllname $[TARGET]].$[dlllib]
  #define sources $($[varname])
  #define flags   $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[CFLAGS_SHARED] $[building_var:%=/D%]
  #define mybasename $[basename $[notdir $[target]]]
  #define tmpdirname_cyg $[install_lib_dir]/$[mybasename]
  #define tmpdirname_win $[osfilename $[tmpdirname_cyg]]

// not parallel (requires gmake 3.79) because of link.exe conflicts in TMP dir (see audiotraits dir)
#if $[GENERATE_BUILDDATE]
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources] $[static_lib_dependencies] $[dtool_ver_dir]/version.rc $[DLLBASEADDRFILENAME:%=$[dtool_ver_dir_cyg]/%]]
// first generate builddate for rc compiler using compiler preprocessor
$[TAB]  if not exist $[osfilename $[tmpdirname_cyg]] mkdir $[osfilename $[tmpdirname_cyg]]  // this dir-creation-stuff is leftover from trying to resolve parallel link difficulties
 #define VER_RESOURCE "$[tmpdirname_win]\$[mybasename].res"
$[TAB]  cl /nologo /EP "$[dtool_ver_dir]\verdate.cpp"  > "$[tmpdirname_win]\verdate.h"
$[TAB]  rc /n /I"$[tmpdirname_win]" $[DECYGWINED_INC_PATHLIST_ARGS] /fo$[VER_RESOURCE] $[filter /D%, $[flags]]  "$[dtool_ver_dir]\version.rc"
  #if $[filter %.cxx %.cpp %.yxx %.lxx,$[get_sources]]
$[TAB] $[SHARED_LIB_C++] $[VER_RESOURCE]
  #else
$[TAB] $[SHARED_LIB_C] $[VER_RESOURCE]
  #endif
#else
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources] $[DLLBASEADDRFILENAME:%=$[dtool_ver_dir_cyg]/%]]
  #if $[filter %.cxx %.cpp %.yxx %.lxx,$[get_sources]]
$[TAB] $[SHARED_LIB_C++]
  #else
$[TAB] $[SHARED_LIB_C]
  #endif
#endif

#if $[build_dlls]
$[osfilename $[ODIR]/$[get_dllname $[TARGET]].lib] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].$[dlllib]]
#endif
#if $[build_pdbs]
$[osfilename $[ODIR]/$[get_dllname $[TARGET]].pdb] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].$[dlllib]]
#endif

#endif

// Here are the rules to install and uninstall the library and
// everything that goes along with it.
#define installed_files \
    $[if $[build_it], \
      $[if $[build_dlls],$[install_lib_dir]/$[get_dllname $[TARGET]].$[dlllib]] \
      $[install_lib_dir]/$[get_dllname $[TARGET]].lib \
      $[if $[and $[build_dlls],$[build_pdbs]],$[install_lib_dir]/$[get_dllname $[TARGET]].pdb] \
    ] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%] \
    $[igatedb:$[ODIR]/%=$[install_igatedb_dir]/%]

install-lib$[TARGET] : $[patsubst %,$[osfilename %],$[installed_files]]

uninstall-lib$[TARGET] :
#if $[installed_files]
#foreach file $[patsubst %,$[osfilename %],$[sort $[installed_files]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

#if $[build_dlls]
$[osfilename $[install_lib_dir]/$[get_dllname $[TARGET]].$[dlllib]] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].$[dlllib]]
#define local $[get_dllname $[TARGET]].$[dlllib]
#define dest $[install_lib_dir]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]
#endif

$[osfilename $[install_lib_dir]/$[get_dllname $[TARGET]].lib] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].lib]
#define local $[get_dllname $[TARGET]].lib
#define dest $[install_lib_dir]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]

#if $[and $[build_dlls],$[build_pdbs]]
$[osfilename $[install_lib_dir]/$[get_dllname $[TARGET]].pdb] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].pdb]
#define local $[get_dllname $[TARGET]].pdb
#define dest $[install_lib_dir]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]
#endif

#if $[igatescan]
// Now, some additional rules to generate and compile the interrogate
// data, if needed.

// The library name is based on this library.
#define igatelib lib$[TARGET]
// The module name comes from the metalib that includes this library.
#define igatemod $[module $[TARGET],$[TARGET]]
#if $[eq $[igatemod],]
  // Unless no metalib includes this library.
  #define igatemod $[TARGET]
#endif

$[osfilename $[igatedb:$[ODIR]/%=$[install_igatedb_dir]/%]] : $[patsubst %,$[osfilename %],$[igatedb]]
#define local $[igatedb]
#define dest $[install_igatedb_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]

// We have to split this out as a separate rule to properly support
// parallel make.
$[osfilename $[igatedb]] : $[patsubst %,$[osfilename %],$[igateoutput]]

lib$[TARGET]_igatescan = $[patsubst %,$[osfilename %],$[igatescan]]
$[osfilename $[igateoutput]] : $[patsubst %,$[osfilename %],$[sort $[patsubst %.h,%.h,%.I,%.I,%.T,%.T,%,,$[dependencies $[igatescan]] $[igatescan:%=./%]]]]
//// Sauce
//// There's a bug here.  The -od is being passed into a string in the file.  This
//// makes slashes look like escape sequences.
//// The hacky fix is to use \\ instead of \.  Windows seems to still let you open files if you
//// include multiple slashes in them.  Then, when quoted, the string will properly 
//// be created.
//$[TAB] $[INTERROGATE] -od $[subst \,\\,$[osfilename $[igatedb]]] -oc $[osfilename $[igateoutput]] $[interrogate_options] -module "$[igatemod]" -library "$[igatelib]" $(lib$[TARGET]_igatescan)
// Actually, drose kindly fixed that
$[TAB] $[INTERROGATE] -od $[osfilename $[igatedb]] -oc $[osfilename $[igateoutput]] $[interrogate_options] -module "$[igatemod]" -library "$[igatelib]" $(lib$[TARGET]_igatescan)

#endif  // igatescan


#if $[igatemout]
// And finally, some additional rules to build the interrogate module
// file into the library, if this is a metalib that includes
// interrogated components.

#define igatelib lib$[TARGET]
#define igatemod $[TARGET]

lib$[TARGET]_igatemscan = $[patsubst %,$[osfilename %],$[igatemscan]]
#define target $[igatemout]
#define sources $(lib$[TARGET]_igatemscan)

$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources]]
$[TAB] $[INTERROGATE_MODULE] -oc $[target] -module "$[igatemod]" -library "$[igatelib]" $[interrogate_module_options] $[sources]

#endif  // igatemout

#end metalib_target lib_target


/////////////////////////////////////////////////////////////////////
// Now, the noninstalled dynamic libraries.  These are presumably used
// only within this directory, or at the most within this tree, and
// also presumably will never include interrogate data.  That, plus
// the fact that we don't need to generate install rules, makes it a
// lot simpler.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_lib_target test_lib_target
#define varname $[subst -,_,lib$[TARGET]_so]
$[varname] = $[patsubst %,$[osfilename $[%_obj]],$[compile_sources]]
#define target $[ODIR]/$[get_dllname $[TARGET]].$[dlllib]
#define sources $($[varname])
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources] $[static_lib_dependencies] $[GENERATED_SOURCES]]
#if $[filter %.cxx %.cpp %.yxx %.lxx,$[get_sources]]
$[TAB] $[SHARED_LIB_C++] $[COMPILED_RESOURCES]
#else
$[TAB] $[SHARED_LIB_C] $[COMPILED_RESOURCES]
#endif

#if $[build_dlls]
$[osfilename $[ODIR]/$[get_dllname $[TARGET]].lib] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].$[dlllib]]
#endif
#if $[build_pdbs]
$[osfilename $[ODIR]/$[get_dllname $[TARGET]].pdb] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].$[dlllib]]
#endif

// this section is all very clunky and not generalized enough
// assuming tgt dirs and such

#define rc_to_gen $[filter %.rc, $[GENERATED_SOURCES]]
#if $[rc_to_gen]
$[osfilename $[rc_to_gen]] : $[patsubst %,$[osfilename %],$[GENERATED_RC_DEPENDENCIES]]
$[TAB] $[RC_GENERATOR_RULE]

$[osfilename $[ODIR]/$[RC_BASENAME].res] : $[patsubst %,$[osfilename %],$[rc_to_gen]]
$[TAB] $[COMPILE_RC] /I"$[ODIR]" /Fo"$[ODIR]/$[RC_BASENAME].res" $[ODIR]/$[RC_BASENAME].rc
#endif

#define inf_to_gen $[filter %.inf, $[GENERATED_SOURCES]]
#if $[inf_to_gen]
$[osfilename $[inf_to_gen]] : $[patsubst %,$[osfilename %],$[GENERATED_INF_DEPENDENCIES]]
$[TAB] $[INF_GENERATOR_RULE]
#endif

#define rgs_to_gen $[filter %.rgs, $[GENERATED_SOURCES]]
#if $[rgs_to_gen]
$[osfilename $[rgs_to_gen]] : $[patsubst %,$[osfilename %],$[GENERATED_RGS_DEPENDENCIES]]
$[TAB] $[RGS_GENERATOR_RULE]
#endif

#define verhdr_to_gen $[filter %Version.h, $[GENERATED_SOURCES]]
#if $[verhdr_to_gen]
$[osfilename $[verhdr_to_gen]] : $[patsubst %,$[osfilename %],$[GENERATED_VERHEADER_DEPENDENCIES]]
$[TAB] $[VERHEADER_GENERATOR_RULE]

$[osfilename $[VERHEADER_DEPENDENTS]] : $[patsubst %,$[osfilename %],$[verhdr_to_gen]]
#endif

#define MIDL_COMMAND $[COMPILE_IDL] /out $[ODIR] $[ODIR]/$[IDL_BASENAME].idl

#define idl_to_gen $[filter %.idl, $[GENERATED_SOURCES]]
#if $[idl_to_gen]
$[osfilename $[idl_to_gen]] : $[patsubst %,$[osfilename %],$[GENERATED_IDL_DEPENDENCIES]]
$[TAB] $[IDL_GENERATOR_RULE]

$[osfilename $[ODIR]/$[IDL_BASENAME].h] : $[patsubst %,$[osfilename %],$[idl_to_gen]]
$[TAB] $[MIDL_COMMAND]

// this is a complete hack.  I dont know how add a generated .h to the dependency list of $[IDL_BASENAME].cpp.
// it is already there, but in the wrong directory.  should really add this to official dependency list
#foreach file $[GENERATED_IDL_H_DEPENDENTS]
$[osfilename $[file]] : $[patsubst %,$[osfilename %],$[ODIR]/$[IDL_BASENAME].h]
$[TAB]  // empty, dependency-only 'rule'

#end file

$[osfilename $[ODIR]/$[IDL_BASENAME].tlb] : $[patsubst %,$[osfilename %],$[idl_to_gen]]
$[TAB] $[MIDL_COMMAND]
#endif

#end noinst_lib_target test_lib_target


/////////////////////////////////////////////////////////////////////
// Now the static libraries.  Again, we assume there's no interrogate
// interfaces going on in here, and there's no question of this being
// a metalib, making the rules relatively simple.
/////////////////////////////////////////////////////////////////////

#forscopes static_lib_target ss_lib_target
#define varname $[subst -,_,lib$[TARGET]_a]
$[varname] = $[patsubst %,$[osfilename $[%_obj]],$[compile_sources]]
#define target $[ODIR]/$[get_dllname $[TARGET]].lib
#define sources $($[varname])
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources]]
#if $[filter %.cxx %.cpp %.yxx %.lxx,$[get_sources]]
$[TAB] $[STATIC_LIB_C++]
#else
$[TAB] $[STATIC_LIB_C]
#endif

#define installed_files \
    $[install_lib_dir]/$[get_dllname $[TARGET]].lib \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-lib$[TARGET] : $[patsubst %,$[osfilename %],$[installed_files]]

uninstall-lib$[TARGET] :
#if $[installed_files]
#foreach file $[patsubst %,$[osfilename %],$[sort $[installed_files]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

$[osfilename $[install_lib_dir]/$[get_dllname $[TARGET]].lib] : $[patsubst %,$[osfilename %],$[ODIR]/$[get_dllname $[TARGET]].lib]
#define local $[get_dllname $[TARGET]].lib
#define dest $[install_lib_dir]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]

#end static_lib_target ss_lib_target



/////////////////////////////////////////////////////////////////////
// The sed_bin_targets are a special bunch.  These are scripts that
// are to be preprocessed with sed before being installed, for
// instance to insert a path or something in an appropriate place.
/////////////////////////////////////////////////////////////////////

#forscopes sed_bin_target
$[osfilename $[TARGET]] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET]]

#define target $[ODIR]/$[TARGET]
#define source $[SOURCE]
#define script $[COMMAND]
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[source]]
$[TAB] $[subst ',",$[SED]]
// $[TAB] chmod +x $[target] // WILL THIS BREAK IN WINDOWS?

#define installed_files \
    $[install_bin_dir]/$[TARGET]

install-$[TARGET] : $[patsubst %,$[osfilename %],$[installed_files]]

uninstall-$[TARGET] :
#if $[installed_files]
#foreach file $[patsubst %,$[osfilename %],$[sort $[installed_files]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

#define local $[TARGET]
#define dest $[install_bin_dir]
$[osfilename $[install_bin_dir]/$[TARGET]] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET]]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]

#end sed_bin_target


/////////////////////////////////////////////////////////////////////
// And now, the bin_targets.  These are normal C++ executables.  No
// interrogate, metalibs, or any such nonsense here.
/////////////////////////////////////////////////////////////////////

#forscopes bin_target
$[osfilename $[TARGET]] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET].exe]

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[patsubst %,$[osfilename $[%_obj]],$[compile_sources]]
#define target $[ODIR]/$[TARGET].exe
#define sources $($[varname])
#define ld $[get_ld]
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources] $[static_lib_dependencies]]
#if $[ld]
  // If there's a custom linker defined for the target, we have to use it.
$[TAB] $[ld] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]
#else
  // Otherwise, we can use the normal linker.
  #if $[filter %.cxx %.cpp %.yxx %.lxx,$[get_sources]]
$[TAB] $[LINK_BIN_C++]
  #else
$[TAB] $[LINK_BIN_C]
  #endif
#endif

#if $[build_pdbs]
$[osfilename $[ODIR]/$[TARGET].pdb] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET].exe]
#endif

#define installed_files \
    $[install_bin_dir]/$[TARGET].exe \
    $[if $[build_pdbs],$[install_bin_dir]/$[TARGET].pdb] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[if $[bin_postprocess_target],$[install_bin_dir]/$[bin_postprocess_target].exe] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-$[TARGET] : $[patsubst %,$[osfilename %],$[installed_files]]

uninstall-$[TARGET] :
#if $[installed_files]
#foreach file $[patsubst %,$[osfilename %],$[sort $[installed_files]]]
$[TAB] if exist $[file] del /f $[file]
#end file
#endif

$[osfilename $[install_bin_dir]/$[TARGET].exe] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET].exe]
#define local $[TARGET].exe
#define dest $[install_bin_dir]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]

#if $[build_pdbs]
$[osfilename $[install_bin_dir]/$[TARGET].pdb] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET].pdb]
#define local $[TARGET].pdb
#define dest $[install_bin_dir]
$[TAB] xcopy /I/Y $[osfilename $[ODIR]/$[local]] $[osfilename $[dest]]
#endif

#if $[bin_postprocess_target]
#define input_exe $[ODIR]/$[TARGET].exe
#define output_exe $[ODIR]/$[bin_postprocess_target].exe

$[osfilename $[output_exe]] : $[patsubst %,$[osfilename %],$[input_exe]]
$[TAB] if exist $[osfilename $[output_exe]] del /f $[osfilename $[output_exe]]
$[TAB] $[bin_postprocess_cmd] $[bin_postprocess_arg1] $[input_exe] $[bin_postprocess_arg2] $[output_exe]
$[TAB] if exist $[file] del /f $[file]

$[osfilename $[install_bin_dir]/$[bin_postprocess_target].exe] : $[patsubst %,$[osfilename %],$[output_exe]]
$[TAB] xcopy /I/Y $[osfilename $[output_exe]] $[osfilename $[install_bin_dir]]
#endif

#end bin_target

/////////////////////////////////////////////////////////////////////
// The noinst_bin_targets and the test_bin_targets share the property
// of being built (when requested), but having no install rules.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_bin_target test_bin_target test_lib_target
$[osfilename $[TARGET]] : $[patsubst %,$[osfilename %],$[ODIR]/$[TARGET].exe]

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[patsubst %,$[osfilename $[%_obj]],$[compile_sources]]
#define target $[ODIR]/$[TARGET].exe
#define sources $($[varname])
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[sources] $[static_lib_dependencies]]
#if $[filter %.cxx %.cpp %.yxx %.lxx,$[get_sources]]
$[TAB] $[LINK_BIN_C++]
#else
$[TAB] $[LINK_BIN_C]
#endif

#end noinst_bin_target test_bin_target test_lib_target

/////////////////////////////////////////////////////////////////////
// Rules to run bison and/or flex as needed.
/////////////////////////////////////////////////////////////////////

// Rules to generate a C++ file from a Bison input file.
#foreach file $[sort $[yxx_st_sources]]
#define target $[patsubst %.yxx,%.cxx,$[file]]
#define target_header $[patsubst %.yxx,%.h,$[file]]
#define target_prebuilt $[target].prebuilt
#define target_header_prebuilt $[target_header].prebuilt
#if $[HAVE_BISON]
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[file]]
$[TAB] $[BISON] $[YFLAGS] -y $[if $[YACC_PREFIX],-d --name-prefix=$[YACC_PREFIX]] $[file]
$[TAB] move /y y.tab.c $[target]
$[TAB] move /y y.tab.h $[target_header]
$[osfilename $[target_header]] : $[patsubst %,$[osfilename %],$[target]]
$[osfilename $[target_prebuilt]] : $[patsubst %,$[osfilename %],$[target]]
$[TAB] copy /Y $[osfilename $[target]] $[osfilename $[target_prebuilt]]
$[osfilename $[target_header_prebuilt]] : $[patsubst %,$[osfilename %],$[target_header]]
$[TAB] copy /Y $[osfilename $[target_header]] $[osfilename $[target_header_prebuilt]]
#else // HAVE_BISON
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[target_prebuilt]]
$[TAB] copy /Y $[osfilename $[target_prebuilt]] $[osfilename $[target]]
$[osfilename $[target_header]] : $[patsubst %,$[osfilename %],$[target_header_prebuilt]]
$[TAB] copy /Y $[osfilename $[target_header_prebuilt]] $[osfilename $[target_header]]
#endif // HAVE_BISON

#end file

// Rules to generate a C++ file from a Flex input file.
#foreach file $[sort $[lxx_st_sources]]
#define target $[patsubst %.lxx,%.cxx,$[file]]
#define target_prebuilt $[target].prebuilt
#if $[HAVE_BISON]
#define source $[file]
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[file]]
$[TAB] $[FLEX] $[LFLAGS] $[if $[YACC_PREFIX],-P$[YACC_PREFIX]] -olex.yy.c $[file]
#define source lex.yy.c
#define script /#include <unistd.h>/d
$[TAB] $[subst ',",$[SED]]
$[TAB] if exist lex.yy.c del /f lex.yy.c
$[osfilename $[target_prebuilt]] : $[patsubst %,$[osfilename %],$[target]]
$[TAB] copy /Y $[osfilename $[target]] $[osfilename $[target_prebuilt]]
#else // HAVE_BISON
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[target_prebuilt]]
$[TAB] copy /Y $[osfilename $[target_prebuilt]] $[osfilename $[target]]
#endif // HAVE_BISON

#end file


/////////////////////////////////////////////////////////////////////
// Finally, we put in the rules to compile each source file into a .obj
// file.
/////////////////////////////////////////////////////////////////////

#forscopes metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target
// need to use #print to avoid printing to Makefile
// printvar prints the unevaluated defn of the var
// #print TARGET=$[TARGET]
// #printvar TARGET

// Rules to compile ordinary C files.
#foreach file $[sort $[c_sources]]
#define target $[$[file]_obj]
#define source $[file]
#define ipath $[target_ipath]
#define flags $[cflags] $[building_var:%=/D%]
#if $[ne $[file], $[notdir $file]]
  // If the source file is not in the current directory, tack on "."
  // to front of the ipath.
  #set ipath . $[ipath]
#endif

$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[source] $[get_depends $[source]]]
$[TAB] $[COMPILE_C]

#end file

// Rules to compile C++ files.

#foreach file $[sort $[cxx_sources]]
#define target $[$[file]_obj]
#define source $[file]
#define ipath $[target_ipath]
#define flags $[c++flags] $[building_var:%=/D%]
#if $[ne $[file], $[notdir $file]]
  // If the source file is not in the current directory, tack on "."
  // to front of the ipath.
  #set ipath . $[ipath]
#endif

// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[osfilename $[target]] : $[patsubst %,$[osfilename %],$[source] $[get_depends $[source]] $[yxx_sources:%.yxx=%.h]]
$[TAB] $[COMPILE_C++]

#end file

#end metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target

// And now the rules to install the auxiliary files, like headers and
// data files.
#foreach file $[install_scripts]
$[osfilename $[install_bin_dir]/$[file]] : $[patsubst %,$[osfilename %],$[file]]
#define local $[file]
#define dest $[install_bin_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#end file

#foreach file $[install_headers]
$[osfilename $[install_headers_dir]/$[file]] : $[patsubst %,$[osfilename %],$[file]]
#define local $[file]
#define dest $[install_headers_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#end file

#foreach file $[install_parser_inc]
#if $[ne $[dir $[file]], ./]
$[osfilename $[install_parser_inc_dir]/$[file]] : $[osfilename $[notdir $[file]]]
  #define local $[notdir $[file]]
  #define dest $[install_parser_inc_dir]/$[dir $[file]]
$[TAB] if not exist $[osfilename $[install_parser_inc_dir]/$[dir $[file]]] mkdir $[osfilename $[install_parser_inc_dir]/$[dir $[file]]] || echo
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#else
$[osfilename $[install_parser_inc_dir]/$[file]] : $[osfilename $[file]]
  #define local $[file]
  #define dest $[install_parser_inc_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#endif
#end file

#foreach file $[install_data]
$[osfilename $[install_data_dir]/$[file]] : $[patsubst %,$[osfilename %],$[file]]
#define local $[file]
#define dest $[install_data_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#end file

#foreach file $[install_config]
$[osfilename $[install_config_dir]/$[file]] : $[patsubst %,$[osfilename %],$[file]]
#define local $[file]
#define dest $[install_config_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#end file

// Finally, all the special targets.  These are commands that just need
// to be invoked; we don't pretend to know what they are.
#forscopes special_target
$[osfilename $[TARGET]] :
$[TAB] $[COMMAND]

#end special_target


// Finally, the rules to freshen the Makefile itself.
Makefile : $[patsubst %,$[osfilename %],$[SOURCE_FILENAME]]
$[TAB] ppremake

#if $[and $[DEPENDENCY_CACHE_FILENAME],$[dep_sources]]
$[osfilename $[DEPENDENCY_CACHE_FILENAME]] : $[patsubst %,$[osfilename %],$[dep_sources]]
$[TAB] @ppremake -D $[DEPENDENCY_CACHE_FILENAME]
#endif

#end Makefile

//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], group]
//////////////////////////////////////////////////////////////////////

// This is a group directory: a directory above a collection of source
// directories, e.g. $DTOOL/src.  We don't need to output anything in
// this directory.



//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], toplevel]
//////////////////////////////////////////////////////////////////////

// This is the toplevel directory, e.g. $DTOOL.  Here we build the
// root makefile and also synthesize the dtool_config.h (or whichever
// file) we need.

#map subdirs
// Iterate through all of our known source files.  Each src and
// metalib type file gets its corresponding Makefile listed
// here.  However, we test for $[DIR_TYPE] of toplevel, because the
// source directories typically don't define their own DIR_TYPE
// variable, and they end up inheriting this one dynamically.
#forscopes */
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib],$[and $[eq $[DIR_TYPE], toplevel],$[ne $[DIRNAME],top]]]
#if $[build_directory]
  #addmap subdirs $[DIRNAME]
#endif
#endif
#end */

#if $[PYTHON_PACKAGE]
#include $[THISDIRPREFIX]PythonPackageInit.pp
#endif


#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

all : $[subdirs]
test : $[subdirs:%=test-%]
igate : $[subdirs:%=igate-%]
clean : $[subdirs:%=clean-%]
clean-igate : $[subdirs:%=clean-igate-%]
cleanall : $[subdirs:%=cleanall-%]
install : $[patsubst %,$[osfilename %],$[if $[CONFIG_HEADER],$[install_headers_dir] $[install_headers_dir]/$[CONFIG_HEADER]] $[subdirs:%=install-%]]
install-igate : $[subdirs:%=install-igate-%]
uninstall : $[subdirs:%=uninstall-%]
#if $[CONFIG_HEADER]
$[TAB] if exist $[osfilename $[install_headers_dir]/$[CONFIG_HEADER]] del /f $[osfilename $[install_headers_dir]/$[CONFIG_HEADER]]
#endif
uninstall-igate : $[subdirs:%=uninstall-igate-%]

#if $[HAVE_BISON]
prebuild-bison : $[subdirs:%=prebuild-bison-%]
clean-prebuild-bison : $[subdirs:%=clean-prebuild-bison-%]
#endif

#formap dirname subdirs
#define depends
$[osfilename $[dirname]] : $[patsubst %,$[osfilename %],$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) all
#end dirname

#formap dirname subdirs
test-$[dirname] :
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) test
#end dirname

#formap dirname subdirs
igate-$[dirname] :
$[TAB]cd $[osfilename ./$[PATH]] && $(MAKE) igate
#end dirname

#formap dirname subdirs
clean-$[dirname] :
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) clean
#end dirname

#formap dirname subdirs
clean-igate-$[dirname] :
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) clean-igate
#end dirname

#formap dirname subdirs
cleanall-$[dirname] : $[patsubst %,$[osfilename %],$[patsubst %,cleanall-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]]
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) cleanall
#end dirname

#formap dirname subdirs
install-$[dirname] : $[patsubst %,$[osfilename %],$[patsubst %,install-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]]
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) install
#end dirname

#formap dirname subdirs
install-igate-$[dirname] :
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) install-igate
#end dirname

#formap dirname subdirs
uninstall-$[dirname] :
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) uninstall
#end dirname

#formap dirname subdirs
uninstall-igate-$[dirname] :
$[TAB] cd $[osfilename ./$[PATH]] && $(MAKE) uninstall-igate
#end dirname

#if $[HAVE_BISON]
#formap dirname subdirs
prebuild-bison-$[dirname] :
$[TAB]cd $[osfilename ./$[PATH]] && $(MAKE) prebuild-bison
clean-prebuild-bison-$[dirname] :
$[TAB]cd $[osfilename ./$[PATH]] && $(MAKE) clean-prebuild-bison
#end dirname
#endif

#if $[ne $[CONFIG_HEADER],]
$[osfilename $[install_headers_dir]] :
$[TAB] if not exist $[osfilename $[install_headers_dir]] echo mkdir $[osfilename $[install_headers_dir]]
$[TAB] if not exist $[osfilename $[install_headers_dir]] mkdir $[osfilename $[install_headers_dir]]

$[osfilename $[install_headers_dir]/$[CONFIG_HEADER]] : $[patsubst %,$[osfilename %],$[CONFIG_HEADER]]
#define local $[CONFIG_HEADER]
#define dest $[install_headers_dir]
$[TAB] xcopy /I/Y $[osfilename $[local]] $[osfilename $[dest]]
#endif

#end Makefile

// If there is a file called LocalSetup.pp in the package's top
// directory, then invoke that.  It might contain some further setup
// instructions.
#sinclude $[TOPDIRPREFIX]LocalSetup.nmake.pp
#sinclude $[TOPDIRPREFIX]LocalSetup.pp


//////////////////////////////////////////////////////////////////////
#elif $[or $[eq $[DIR_TYPE], models],$[eq $[DIR_TYPE], models_toplevel],$[eq $[DIR_TYPE], models_group]]
//////////////////////////////////////////////////////////////////////

#include $[THISDIRPREFIX]Template.models.pp

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE


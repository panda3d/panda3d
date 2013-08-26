//
//
// Template.unix.pp
//
// This file defines the set of output files that will be generated to
// support a generic Unix-style build system.
//

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
// $DTOOL/pptempl/Global.gmsvc.pp
// $DTOOL/pptempl/Depends.pp, once for each Sources.pp file
// Template.gmsvc.pp (this file), once for each Sources.pp file

#if $[ne $[DTOOL],]
#define dtool_ver_dir_cyg $[DTOOL]/src/dtoolbase
#define dtool_ver_dir $[osfilename $[dtool_ver_dir_cyg]]
#endif

//////////////////////////////////////////////////////////////////////
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib]]
//////////////////////////////////////////////////////////////////////
// For a source directory, build a single Makefile with rules to build
// each target.

#if $[build_directory]
  // We need to know the various targets we'll be building.
  // $[lib_targets] will be the list of dynamic and static libraries,
  // and $[bin_targets] the list of binaries.  $[test_bin_targets] is
  // the list of binaries that are to be built only when specifically
  // asked for.
  #define lib_targets $[active_target_libprefext(metalib_target lib_target static_lib_target dynamic_lib_target ss_lib_target noinst_lib_target):%=$[ODIR]/%]
  #define bundle_targets $[active_target_bundleext(metalib_target):%=$[ODIR]/%]
 
  #define bin_targets $[active_target(bin_target noinst_bin_target sed_bin_target):%=$[ODIR]/%]
  #define test_bin_targets $[active_target(test_bin_target):%=$[ODIR]/%]

  // And these variables will define the various things we need to
  // install.
  #define install_lib $[active_target(metalib_target lib_target ss_lib_target static_lib_target)]
  #define install_bin $[active_target(bin_target)]
  #define install_scripts $[sort $[INSTALL_SCRIPTS(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_SCRIPTS]]
  #define install_modules $[sort $[INSTALL_MODULES(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_MODULES]]
  #define install_headers $[sort $[INSTALL_HEADERS(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_HEADERS]]
  #define install_parser_inc $[sort $[INSTALL_PARSER_INC]]
  #define install_data $[sort $[INSTALL_DATA(metalib_target lib_target ss_lib_target static_lib_target dynamic_lib_target bin_target)] $[INSTALL_DATA]]
  #define install_config $[sort $[INSTALL_CONFIG(metalib_target lib_target ss_lib_target static_lib_target dynamic_lib_target bin_target)] $[INSTALL_CONFIG]]
  #define install_igatedb $[sort $[get_igatedb(metalib_target lib_target ss_lib_target)]]

  // These are the various sources collected from all targets within the
  // directory.
  #define st_sources $[sort $[compile_sources(metalib_target lib_target noinst_lib_target static_lib_target dynamic_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)]]
  #define yxx_st_sources $[sort $[yxx_sources(metalib_target lib_target noinst_lib_target static_lib_target dynamic_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)]]
  #define lxx_st_sources $[sort $[lxx_sources(metalib_target lib_target noinst_lib_target static_lib_target dynamic_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)]]
  #define dep_sources_1 $[sort $[get_sources(metalib_target lib_target noinst_lib_target static_lib_target dynamic_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)]]
  
  // These are the source files that our dependency cache file will
  // depend on.  If it's an empty list, we won't bother writing rules to
  // freshen the cache file.
  #define dep_sources $[sort $[filter %.c %.cxx %.mm %.yxx %.lxx %.h %.I %.T,$[dep_sources_1]]]

  // If there is an __init__.py in the directory, then all Python
  // files in the directory just get installed without having to be
  // named.
  #if $[and $[INSTALL_PYTHON_SOURCE],$[wildcard $[TOPDIR]/$[DIRPREFIX]__init__.py]]
    #define py_sources $[wildcard $[TOPDIR]/$[DIRPREFIX]*.py]
  #endif
  #define install_py $[py_sources:$[TOPDIR]/$[DIRPREFIX]%=%]

#endif  // $[build_directory]

#defer actual_local_libs $[complete_local_libs]

// $[static_lib_dependencies] is the set of libraries we will link
// with that happen to be static libs.  We will introduce dependency
// rules for these.  (We don't need dependency rules for dynamic libs,
// since these don't get burned in at build time.)
#defer static_lib_dependencies $[all_libs $[if $[lib_is_static],$[RELDIR:%=%/$[ODIR]/$[lib_prefix]$[TARGET]$[dllext]$[lib_ext]]],$[complete_local_libs]]

// $[target_ipath] is the proper ipath to put on the command line,
// from the context of a particular target.

#defer target_ipath $[TOPDIR] $[sort $[complete_ipath]] $[other_trees_include] $[get_ipath]

// These are the complete set of extra flags the compiler requires.
#defer cflags $[get_cflags] $[CFLAGS] $[CFLAGS_OPT$[OPTIMIZE]] 
#defer c++flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] 
#defer lflags $[get_lflags] $[LFLAGS] $[LFLAGS_OPT$[OPTIMIZE]] 

// $[complete_lpath] is rather like $[complete_ipath]: the list of
// directories (from within this tree) we should add to our -L list.
#defer complete_lpath $[libs $[RELDIR:%=%/$[ODIR]],$[actual_local_libs]] $[EXTRA_LPATH]

// $[lpath] is like $[target_ipath]: it's the list of directories we
// should add to our -L list, from the context of a particular target.
#defer lpath $[sort $[complete_lpath]] $[other_trees_lib] $[install_lib_dir] $[get_lpath]

// And $[libs] is the set of libraries we will link with.
#defer nonunique_libs $[nonunique_complete_local_libs:%=%$[dllext]] $[patsubst %:m,,%:c %,%$[dllext],$[OTHER_LIBS]] $[get_libs]

// Don't use $[unique] here, since some libraries actually do need to be
// named multiple times (when linking static).
#if $[LINK_ALL_STATIC]
  #defer libs $[nonunique_libs]
#else
  #defer libs $[unique $[nonunique_libs]]
#endif

// And $[frameworks] is the set of OSX-style frameworks we will link with.
#defer frameworks $[unique $[get_frameworks]]
#defer bin_frameworks $[unique $[get_frameworks] $[all_libs $[get_frameworks],$[complete_local_libs]]]
//#defer bin_frameworks $[unique $[get_frameworks]]

// This is the set of files we might copy into *.prebuilt, if we have
// bison and flex (or copy from *.prebuilt if we don't have them).
#define bison_prebuilt $[patsubst %.yxx,%.cxx %.h,$[yxx_st_sources]] $[patsubst %.lxx,%.cxx,$[lxx_st_sources]]

// Rather than making a rule to generate each install directory later,
// we create the directories now.  This reduces problems from
// multiprocess builds.
#mkdir $[sort \
    $[if $[install_lib],$[install_lib_dir]] \
    $[if $[install_bin] $[install_scripts],$[install_bin_dir]] \
    $[if $[install_lib] $[install_modules],$[install_lib_dir]] \
    $[if $[install_headers],$[install_headers_dir]] \
    $[if $[install_parser_inc],$[install_parser_inc_dir]] \
    $[if $[install_data],$[install_data_dir]] \
    $[if $[install_config],$[install_config_dir]] \
    $[if $[install_igatedb],$[install_igatedb_dir]] \
    $[if $[install_py],$[install_py_dir] $[install_py_package_dir]] \
    ]

// Similarly, we need to ensure that $[ODIR] exists.  Trying to make
// the makefiles do this automatically just causes problems with
// multiprocess builds.
#mkdir $[ODIR]

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
#if $[USE_TAU]
// For the benefit of Tau, we copy the source file verbatim into the
// composite file.  (Tau doesn't instrument files picked up via #include.)
#copy $[DIRPREFIX]$[file]

#else
##include "$[file]"
#endif  // USE_TAU
#end file

#end $[composite_file]
#end composite_file

// Okay, we're ready.  Start outputting the Makefile now.
#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#if $[and $[USE_TAU],$[TAU_MAKEFILE]]
include $[TAU_MAKEFILE]
#endif

// If we are using GNU make, this will automatically enable the
// multiprocessor build mode according to the value in
// NUMBER_OF_PROCESSORS, which should be set by NT.  Maybe this isn't
// a good idea to do all the time, but you can always disable it by
// explicitly unsetting NUMBER_OF_PROCESSORS, or by setting it to 1.
//#if $[NUMBER_OF_PROCESSORS]
//MAKEFLAGS := -j$[NUMBER_OF_PROCESSORS]
//#endif

// The 'all' rule makes all the stuff in the directory except for the
// test_bin_targets.  It doesn't do any installation, however.
#define all_targets \
    Makefile \
    $[if $[dep_sources],$[DEPENDENCY_CACHE_FILENAME]] \
    $[sort $[lib_targets] $[bundle_targets] $[bin_targets]]
all : $[all_targets]

// The 'test' rule makes all the test_bin_targets.
test : $[test_bin_targets]

clean : clean-igate
#forscopes metalib_target lib_target noinst_lib_target static_lib_target dynamic_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target
#if $[compile_sources]
$[TAB] rm -f $[patsubst %,$[%_obj],$[compile_sources]]
#endif
#end metalib_target lib_target noinst_lib_target static_lib_target dynamic_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target test_lib_target
#if $[lib_targets] $[bundle_targets] $[bin_targets] $[test_bin_targets]
$[TAB] rm -f $[lib_targets] $[bundle_targets] $[bin_targets] $[test_bin_targets]
#endif
#if $[yxx_st_sources] $[lxx_st_sources]
$[TAB] rm -f $[patsubst %.yxx,%.cxx %.h,$[yxx_st_sources]] $[patsubst %.lxx,%.cxx,$[lxx_st_sources]]
#endif
#if $[py_sources]
$[TAB] rm -f *.pyc *.pyo // Also scrub out old generated Python code.
#endif
#if $[USE_TAU]
$[TAB] rm -f *.pdb *.inst.*  // scrub out tau-generated files.
#endif
                         
// 'cleanall' is intended to undo all the effects of running ppremake
// and building.  It removes everything except the Makefile.
cleanall : clean
#if $[st_sources]
$[TAB] rm -rf $[ODIR]
#endif
#if $[ne $[DEPENDENCY_CACHE_FILENAME],]
$[TAB] rm -f $[DEPENDENCY_CACHE_FILENAME]
#endif
#if $[composite_list]
$[TAB] rm -f $[composite_list]
#endif

clean-igate :
#forscopes metalib_target lib_target ss_lib_target dynamic_lib_target
  #define igatedb $[get_igatedb]
  #define igateoutput $[get_igateoutput]
  #define igatemscan $[get_igatemscan]
  #define igatemout $[get_igatemout]
  #if $[igatedb]
$[TAB] rm -f $[igatedb]
  #endif
  #if $[igateoutput]
$[TAB] rm -f $[igateoutput] $[$[igateoutput]_obj]
  #endif
  #if $[igatemout]
$[TAB] rm -f $[igatemout] $[$[igatemout]_obj]
  #endif
#end metalib_target lib_target ss_lib_target dynamic_lib_target

// Now, 'install' and 'uninstall'.  These simply copy files into the
// install directory (or remove them).  The 'install' rule also makes
// the directories if necessary.
#define installed_files \
     $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
     $[INSTALL_MODULES:%=$[install_lib_dir]/%] \
     $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
     $[INSTALL_PARSER_INC:%=$[install_parser_inc_dir]/%] \
     $[INSTALL_DATA:%=$[install_data_dir]/%] \
     $[INSTALL_CONFIG:%=$[install_config_dir]/%] \
     $[if $[install_py],$[install_py:%=$[install_py_dir]/%] $[install_py_package_dir]/__init__.py]

#define installed_igate_files \
     $[get_igatedb(metalib_target lib_target ss_lib_target):$[ODIR]/%=$[install_igatedb_dir]/%]

#define install_targets \
     $[active_target(metalib_target lib_target static_lib_target dynamic_lib_target ss_lib_target):%=install-lib%] \
     $[active_target(bin_target sed_bin_target):%=install-%] \
     $[installed_files]

install : all $[install_targets]

install-igate : $[sort $[installed_igate_files]]

uninstall : $[active_target(metalib_target lib_target static_lib_target dynamic_lib_target ss_lib_target):%=uninstall-lib%] $[active_target(bin_target):%=uninstall-%]
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

uninstall-igate :
#if $[installed_igate_files]
$[TAB] rm -f $[sort $[installed_igate_files]]
#endif

#if $[HAVE_BISON]
prebuild-bison : $[patsubst %,%.prebuilt,$[bison_prebuilt]]
clean-prebuild-bison : 
#if $[bison_prebuilt]
$[TAB] rm -f $[sort $[patsubst %,%.prebuilt,$[bison_prebuilt]]]
#endif
#endif

// Now it's time to start generating the rules to make our actual
// targets.

igate : $[get_igatedb(metalib_target lib_target ss_lib_target)]


/////////////////////////////////////////////////////////////////////
// First, the normally installed dynamic and static libraries.
/////////////////////////////////////////////////////////////////////

#forscopes metalib_target lib_target ss_lib_target static_lib_target dynamic_lib_target

// In Unix, we always build all the libraries, unlike Windows.
#define build_it 1

// We don't need a BUILDING_ symbol for Unix; that's a Windows thing.
#define building_var

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
   $[patsubst %,$[%_obj],$[c_sources] $[mm_sources] $[cxx_sources]]
  #define interrogate_sources \
   $[patsubst %,$[%_obj],$[cxx_interrogate_sources]]
  #define cc_ld $[or $[get_ld],$[CC]]
  #define cxx_ld $[or $[get_ld],$[CXX]]

  #define varname $[subst -,_,.,_,$[lib_prefix]$[TARGET]$[lib_ext]]
$[varname] = $[sources] $[if $[not $[link_extra_bundle]],$[interrogate_sources]]
  #define target $[ODIR]/$[lib_prefix]$[TARGET]$[lib_ext]
  #define sources $($[varname])

$[target] : $[sources] $[static_lib_dependencies]
  #if $[filter %.mm %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[link_lib_c++]
  #else  
$[TAB] $[link_lib_c]
  #endif

  #if $[link_extra_bundle]
    // Also generate the bundles (on OSX only).
    #define target $[ODIR]/$[lib_prefix]$[TARGET]$[bundle_ext]
    #define sources $[interrogate_sources] $[ODIR]/$[lib_prefix]$[TARGET]$[lib_ext]
$[target] : $[sources] $[static_lib_dependencies] 
$[TAB] $[BUNDLE_LIB_C++]
  #endif  // BUNDLE_EXT
#endif

// Here are the rules to install and uninstall the library and
// everything that goes along with it.
#define installed_files \
    $[install_lib_dir]/$[lib_prefix]$[TARGET]$[lib_ext] \
    $[if $[link_extra_bundle],$[install_lib_dir]/$[lib_prefix]$[TARGET]$[bundle_ext]] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_MODULES:%=$[install_lib_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%] \
    $[igatedb:$[ODIR]/%=$[install_igatedb_dir]/%]

install-lib$[TARGET] : $[installed_files]

uninstall-lib$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

$[install_lib_dir]/$[lib_prefix]$[TARGET]$[lib_ext] : $[ODIR]/$[lib_prefix]$[TARGET]$[lib_ext]
#define local $[ODIR]/$[lib_prefix]$[TARGET]$[lib_ext]
#define dest $[install_lib_dir]
$[TAB] $[INSTALL_PROG]

#if $[link_extra_bundle]
$[install_lib_dir]/$[lib_prefix]$[TARGET]$[bundle_ext] : $[ODIR]/$[lib_prefix]$[TARGET]$[bundle_ext]
#define local $[ODIR]/$[lib_prefix]$[TARGET]$[bundle_ext]
#define dest $[install_lib_dir]
$[TAB] $[INSTALL_PROG]
#endif  // link_extra_bundle

#if $[igatescan]
// Now, some additional rules to generate and compile the interrogate
// data, if needed.

// The library name is based on this library.
#define igatelib $[lib_prefix]$[TARGET]
// The module name comes from the metalib that includes this library.
#define igatemod $[module $[TARGET],$[TARGET]]
#if $[eq $[igatemod],]
  // Unless no metalib includes this library.
  #define igatemod $[TARGET]
#endif

$[igatedb:$[ODIR]/%=$[install_igatedb_dir]/%] : $[igatedb]
#define local $[igatedb]
#define dest $[install_igatedb_dir]
$[TAB] $[INSTALL]

// We have to split this out as a separate rule to properly support
// parallel make.
$[igatedb] : $[igateoutput]

$[lib_prefix]$[TARGET]_igatescan = $[igatescan]
$[igateoutput] : $[sort $[patsubst %.h,%.h,%.I,%.I,%.T,%.T,%,,$[dependencies $[igatescan]] $[igatescan:%=./%]]]
$[TAB] $[INTERROGATE] -od $[igatedb] -oc $[igateoutput] $[interrogate_options] -module "$[igatemod]" -library "$[igatelib]" $($[lib_prefix]$[TARGET]_igatescan)

#endif  // igatescan


#if $[igatemout]
// And finally, some additional rules to build the interrogate module
// file into the library, if this is a metalib that includes
// interrogated components.

#define igatelib $[lib_prefix]$[TARGET]
#define igatemod $[TARGET]

$[lib_prefix]$[TARGET]_igatemscan = $[igatemscan]
#define target $[igatemout]
#define sources $($[lib_prefix]$[TARGET]_igatemscan)

$[target] : $[sources]
$[TAB] $[INTERROGATE_MODULE] -oc $[target] -module "$[igatemod]" -library "$[igatelib]" $[interrogate_module_options] $[sources]

#endif  // igatemout

#end metalib_target lib_target ss_lib_target static_lib_target dynamic_lib_target




/////////////////////////////////////////////////////////////////////
// Now, the noninstalled dynamic libraries.  These are presumably used
// only within this directory, or at the most within this tree, and
// also presumably will never include interrogate data.  That, plus
// the fact that we don't need to generate install rules, makes it a
// lot simpler.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_lib_target
#define varname $[subst -,_,$[lib_prefix]$[TARGET]_so]
$[varname] = $[patsubst %,$[%_obj],$[compile_sources]]
#define target $[ODIR]/$[lib_prefix]$[TARGET]$[lib_ext]
#define sources $($[varname])
$[target] : $[sources] $[static_lib_dependencies]
#if $[filter %.mm %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[link_lib_c++]
#else
$[TAB] $[link_lib_c]
#endif

#end noinst_lib_target


/////////////////////////////////////////////////////////////////////
// The sed_bin_targets are a special bunch.  These are scripts that
// are to be preprocessed with sed before being installed, for
// instance to insert a path or something in an appropriate place.
/////////////////////////////////////////////////////////////////////

#forscopes sed_bin_target
$[TARGET] : $[ODIR]/$[TARGET]

#define target $[ODIR]/$[TARGET]
#define source $[SOURCE]
#define script $[COMMAND]
$[target] : $[source]
$[TAB] $[SED]
$[TAB] chmod +x $[target]

#define installed_files \
    $[install_bin_dir]/$[TARGET]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

#define local $[ODIR]/$[TARGET]
#define dest $[install_bin_dir]
$[install_bin_dir]/$[TARGET] : $[ODIR]/$[TARGET]
$[TAB] $[INSTALL_PROG]

#end sed_bin_target


/////////////////////////////////////////////////////////////////////
// And now, the bin_targets.  These are normal C++ executables.  No
// interrogate, metalibs, or any such nonsense here.
/////////////////////////////////////////////////////////////////////

#forscopes bin_target
$[TARGET] : $[ODIR]/$[TARGET]

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[patsubst %,$[%_obj],$[compile_sources]]
#define target $[ODIR]/$[TARGET]
#define sources $($[varname])
#define cc_ld $[or $[get_ld],$[CC]]
#define cxx_ld $[or $[get_ld],$[CXX]]
#define flags $[lflags]
$[target] : $[sources] $[static_lib_dependencies]
#if $[filter %.mm %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[link_bin_c++]
#else
$[TAB] $[link_bin_c]
#endif

#define installed_files \
    $[install_bin_dir]/$[TARGET] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_MODULES:%=$[install_lib_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

$[install_bin_dir]/$[TARGET] : $[ODIR]/$[TARGET]
#define local $[ODIR]/$[TARGET]
#define dest $[install_bin_dir]
$[TAB] $[INSTALL_PROG]

#end bin_target



/////////////////////////////////////////////////////////////////////
// The noinst_bin_targets and the test_bin_targets share the property
// of being built (when requested), but having no install rules.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_bin_target test_bin_target
$[TARGET] : $[ODIR]/$[TARGET]

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[patsubst %,$[%_obj],$[compile_sources]]
#define target $[ODIR]/$[TARGET]
#define sources $($[varname])
#define cc_ld $[or $[get_ld],$[CC]]
#define cxx_ld $[or $[get_ld],$[CXX]]
#define flags $[lflags]
$[target] : $[sources] $[static_lib_dependencies]
#if $[filter %.mm %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[link_bin_c++]
#else
$[TAB] $[link_bin_c]
#endif

#end noinst_bin_target test_bin_target



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
$[target] : $[file]
$[TAB] $[BISON] $[YFLAGS] -y $[if $[YACC_PREFIX],-d --name-prefix=$[YACC_PREFIX]] $[file]
$[TAB] mv y.tab.c $[target]
$[TAB] mv y.tab.h $[target_header]
$[target_header] : $[target]
$[target_prebuilt] : $[target]
$[TAB] cp $[target] $[target_prebuilt]
$[target_header_prebuilt] : $[target_header]
$[TAB] cp $[target_header] $[target_header_prebuilt]
#else // HAVE_BISON
$[target] : $[target_prebuilt]
$[TAB] cp $[target_prebuilt] $[target]
$[target_header] : $[target_header_prebuilt]
$[TAB] cp $[target_header_prebuilt] $[target_header]
#endif // HAVE_BISON

#end file

// Rules to generate a C++ file from a Flex input file.
#foreach file $[sort $[lxx_st_sources]]
#define target $[patsubst %.lxx,%.cxx,$[file]]
#define target_prebuilt $[target].prebuilt
#if $[HAVE_BISON]
#define source $[file]
$[target] : $[file]
$[TAB] $[FLEX] $[FLEXFLAGS] $[if $[YACC_PREFIX],-P$[YACC_PREFIX]] -olex.yy.c $[file]
#define source lex.yy.c
#define script /#include <unistd.h>/d
$[TAB] $[SED]
$[TAB] rm lex.yy.c
$[target_prebuilt] : $[target]
$[TAB] cp $[target] $[target_prebuilt]
#else // HAVE_BISON
$[target] : $[target_prebuilt]
$[TAB] cp $[target_prebuilt] $[target]
#endif // HAVE_BISON

#end file


/////////////////////////////////////////////////////////////////////
// Finally, we put in the rules to compile each source file into a .obj
// file.
/////////////////////////////////////////////////////////////////////

#forscopes static_lib_target bin_target noinst_bin_target test_bin_target

// Rules to compile ordinary C files (static objects).
#foreach file $[sort $[c_sources]]
#define target $[$[file]_obj]
#define source $[file]
#define ipath $[target_ipath]
#define flags $[cflags]
#if $[ne $[file], $[notdir $file]]
  // If the source file is not in the current directory, tack on "."
  // to front of the ipath.
  #set ipath . $[ipath]
#endif

$[target] : $[source] $[get_depends $[source]]
$[TAB] $[compile_c]

#end file

// Rules to compile C++ files (static objects).

#foreach file $[sort $[mm_sources] $[cxx_sources] $[cxx_interrogate_sources]]
#define target $[$[file]_obj]
#define source $[file]
#define ipath $[target_ipath]
#define flags $[c++flags]
#if $[ne $[file], $[notdir $file]]
  // If the source file is not in the current directory, tack on "."
  // to front of the ipath.
  #set ipath . $[ipath]
#endif

// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target] : $[source] $[get_depends $[source]] $[yxx_sources:%.yxx=%.h]
$[TAB] $[compile_c++]

#end file

#end static_lib_target bin_target noinst_bin_target test_bin_target

#forscopes metalib_target lib_target noinst_lib_target ss_lib_target

// Rules to compile ordinary C files (shared objects).
#foreach file $[sort $[c_sources]]
#define target $[$[file]_obj]
#define source $[file]
#define ipath $[target_ipath]
#define flags $[cflags] $[CFLAGS_SHARED]
#if $[ne $[file], $[notdir $file]]
  // If the source file is not in the current directory, tack on "."
  // to front of the ipath.
  #set ipath . $[ipath]
#endif

$[target] : $[source] $[get_depends $[source]]
$[TAB] $[compile_c]

#end file

// Rules to compile C++ files (shared objects).

#foreach file $[sort $[mm_sources] $[cxx_sources] $[cxx_interrogate_sources]]
#define target $[$[file]_obj]
#define source $[file]
#define ipath $[target_ipath]
#define flags $[c++flags] $[CFLAGS_SHARED]
#if $[ne $[file], $[notdir $file]]
  // If the source file is not in the current directory, tack on "."
  // to front of the ipath.
  #set ipath . $[ipath]
#endif

// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target] : $[source] $[get_depends $[source]] $[yxx_sources:%.yxx=%.h]
$[TAB] $[compile_c++]

#end file

#end metalib_target lib_target noinst_lib_target ss_lib_target

// And now the rules to install the auxiliary files, like headers and
// data files.
#foreach file $[install_scripts]
$[install_bin_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_bin_dir]
$[TAB] $[INSTALL_PROG]
#end file

#foreach file $[install_modules]
$[install_lib_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_lib_dir]
$[TAB] $[INSTALL_PROG]
#end file

#foreach file $[install_headers]
$[install_headers_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_headers_dir]
$[TAB] $[INSTALL]
#end file

#foreach file $[install_parser_inc]
#if $[ne $[dir $[file]], ./]
$[install_parser_inc_dir]/$[file] : $[notdir $[file]]
  #define local $[notdir $[file]]
  #define dest $[install_parser_inc_dir]/$[dir $[file]]
$[TAB] mkdir -p $[install_parser_inc_dir]/$[dir $[file]] || echo
$[TAB] $[INSTALL]
#else
$[install_parser_inc_dir]/$[file] : $[file]
  #define local $[file]
  #define dest $[install_parser_inc_dir]
$[TAB] $[INSTALL]
#endif
#end file

#foreach file $[install_data]
$[install_data_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_data_dir]
$[TAB] $[INSTALL]
#end file

#foreach file $[install_config]
$[install_config_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_config_dir]
$[TAB] $[INSTALL]
#end file

#foreach file $[install_py]
$[install_py_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_py_dir]
$[TAB] $[INSTALL]
#end file

#if $[install_py]
$[install_py_package_dir]/__init__.py :
$[TAB] touch $[install_py_package_dir]/__init__.py
#endif

// Finally, all the special targets.  These are commands that just need
// to be invoked; we don't pretend to know what they are.
#forscopes special_target
$[TARGET] :
$[TAB] $[COMMAND]

#end special_target


// Finally, the rules to freshen the Makefile itself.
Makefile : $[SOURCE_FILENAME] $[EXTRA_PPREMAKE_SOURCE]
$[TAB] ppremake

#if $[USE_TAU]
#foreach composite_file $[composite_list]
$[composite_file] : $[$[composite_file]_sources]
$[TAB] ppremake
#end composite_file
#endif   // USE_TAU

#if $[and $[DEPENDENCY_CACHE_FILENAME],$[dep_sources]]
$[DEPENDENCY_CACHE_FILENAME] : $[dep_sources]
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
install : $[if $[CONFIG_HEADER],$[install_headers_dir] $[install_headers_dir]/$[CONFIG_HEADER]] $[subdirs:%=install-%]
install-igate : $[subdirs:%=install-igate-%]
uninstall : $[subdirs:%=uninstall-%]
#if $[CONFIG_HEADER]
$[TAB] rm -f $[install_headers_dir]/$[CONFIG_HEADER]
#endif
uninstall-igate : $[subdirs:%=uninstall-igate-%]

#if $[HAVE_BISON]
prebuild-bison : $[subdirs:%=prebuild-bison-%]
clean-prebuild-bison : $[subdirs:%=clean-prebuild-bison-%]
#endif

#formap dirname subdirs
#define depends 
$[dirname] : $[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]
$[TAB] cd ./$[PATH] && $(MAKE) all
#end dirname

#formap dirname subdirs
test-$[dirname] :
$[TAB] cd ./$[PATH] && $(MAKE) test
#end dirname

#formap dirname subdirs
igate-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) igate
#end dirname

#formap dirname subdirs
clean-$[dirname] :
$[TAB] cd ./$[PATH] && $(MAKE) clean
#end dirname

#formap dirname subdirs
clean-igate-$[dirname] :
$[TAB] cd ./$[PATH] && $(MAKE) clean-igate
#end dirname

#formap dirname subdirs
cleanall-$[dirname] : $[patsubst %,cleanall-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]
$[TAB] cd ./$[PATH] && $(MAKE) cleanall
#end dirname

#formap dirname subdirs
install-$[dirname] : $[patsubst %,install-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]
$[TAB] cd ./$[PATH] && $(MAKE) install
#end dirname

#formap dirname subdirs
install-igate-$[dirname] :
$[TAB] cd ./$[PATH] && $(MAKE) install-igate
#end dirname

#formap dirname subdirs
uninstall-$[dirname] :
$[TAB] cd ./$[PATH] && $(MAKE) uninstall
#end dirname

#formap dirname subdirs
uninstall-igate-$[dirname] :
$[TAB] cd ./$[PATH] && $(MAKE) uninstall-igate
#end dirname

#if $[HAVE_BISON]
#formap dirname subdirs
prebuild-bison-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) prebuild-bison
clean-prebuild-bison-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) clean-prebuild-bison
#end dirname
#endif

#if $[ne $[CONFIG_HEADER],]
$[install_headers_dir] :
$[TAB] @test -d $[install_headers_dir] || echo mkdir -p $[install_headers_dir]
$[TAB] @test -d $[install_headers_dir] || mkdir -p $[install_headers_dir]

$[install_headers_dir]/$[CONFIG_HEADER] : $[CONFIG_HEADER]
#define local $[CONFIG_HEADER]
#define dest $[install_headers_dir]
$[TAB] $[INSTALL]
#endif

// Finally, the rules to freshen the Makefile itself.
Makefile : $[SOURCE_FILENAME] $[EXTRA_PPREMAKE_SOURCE]
$[TAB] ppremake

#end Makefile

// If there is a file called LocalSetup.pp in the package's top
// directory, then invoke that.  It might contain some further setup
// instructions.
#sinclude $[TOPDIRPREFIX]LocalSetup.unix.pp
#sinclude $[TOPDIRPREFIX]LocalSetup.pp


//////////////////////////////////////////////////////////////////////
#elif $[or $[eq $[DIR_TYPE], models],$[eq $[DIR_TYPE], models_toplevel],$[eq $[DIR_TYPE], models_group]]
//////////////////////////////////////////////////////////////////////

#include $[THISDIRPREFIX]Template.models.pp

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE

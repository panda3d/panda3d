//
// Template.gmsvc.pp
//
// This file defines the set of output files that will be generated to
// support a makefile build system invoking Microsoft's Visual C++
// command-line compiler, similar to Template.msvc.pp, but using
// Cygwin's GNU make instead of Microsoft's nmake.
//

// Before this file is processed, the following files are read and
// processed (in order):

// The Package.pp file in the root of the current source hierarchy
//   (e.g. $PANDA/Package.pp)
// $DTOOL/Package.pp
// $DTOOL/Config.pp
// $DTOOL/Config.Platform.pp
// $DTOOL/pptempl/System.pp
// The user's PPREMAKE_CONFIG file.
// $DTOOL/pptempl/Global.pp
// $DTOOL/pptempl/Global.gmsvc.pp
// All of the Sources.pp files in the current source hierarchy
// $DTOOL/Depends.pp, once for each Sources.pp file
// Template.gmsvc.pp (this file), once for each Sources.pp file

#defun decygwin frompat,topat,path
  #foreach file $[path]
    #if $[isfullpath $[file]]
      $[patsubstw $[frompat],$[topat],$[cygpath_w $[file]]]
    #else
      $[patsubstw $[frompat],$[topat],$[osfilename $[file]]]
    #endif
  #end file
#end decygwin

#define dtool_ver_dir_cyg $[DTOOL_INSTALL]/src/dtoolbase
#define dtool_ver_dir $[decygwin %,%,$[dtool_ver_dir_cyg]]

//////////////////////////////////////////////////////////////////////
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib]]
//////////////////////////////////////////////////////////////////////
// For a source directory, build a single Makefile with rules to build
// each target.

// This is the real set of lib_targets we'll be building.  On Windows,
// we don't build the shared libraries which are included on metalibs.
#define real_lib_targets
#define deferred_objs
#forscopes lib_target
  #if $[eq $[module $[TARGET],$[TARGET]],]
    // This library is not on a metalib, so we can build it.
    #set real_lib_targets $[real_lib_targets] $[TARGET]
  #else
    // This library is on a metalib, so we can't build it, but we
    // should build all the obj's that go into it.
    #set deferred_objs $[deferred_objs] \
      $[patsubst %_src.cxx,,%.c %.cxx %.yxx %.lxx,$[so_dir]/%.obj,%,,$[get_sources] $[get_igateoutput]]
  #endif
#end lib_target

// We need to know the various targets we'll be building.
// $[lib_targets] will be the list of dynamic libraries,
// $[static_lib_targets] the list of static libraries, and
// $[bin_targets] the list of binaries.  $[test_bin_targets] is the
// list of binaries that are to be built only when specifically asked
// for.
#define lib_targets $[patsubst %,$[so_dir]/lib%$[dllext].$[dlllib],$[active_target(metalib_target noinst_lib_target)] $[real_lib_targets]]
#define static_lib_targets $[active_target(static_lib_target ss_lib_target):%=$[st_dir]/lib%$[dllext].lib]
#define bin_targets \
    $[active_target(bin_target noinst_bin_target):%=$[st_dir]/%.exe] \
    $[active_target(sed_bin_target):%=$[st_dir]/%]
#define test_bin_targets $[active_target(test_bin_target):%=$[st_dir]/%.exe]

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

// $[so_sources] is the set of sources that belong on a shared object,
// and $[st_sources] is the set of sources that belong on a static
// object, like a static library or an executable.  In Windows, we
// don't need to make this distinction, but we do anyway in case we
// might in the future for some nutty reason.
#define so_sources $[get_sources(metalib_target lib_target noinst_lib_target)]
#define st_sources $[get_sources(static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)]

// These are the source files that our dependency cache file will
// depend on.  If it's an empty list, we won't bother writing rules to
// freshen the cache file.
#define dep_sources $[sort $[filter %.c %.cxx %.yxx %.lxx %.h %.I %.T,$[so_sources] $[st_sources]]]

#if $[eq $[so_dir],$[st_dir]]
  // If the static and shared directories are the same, we have to use the
  // same rules to build both shared and static targets.
  #set st_sources $[so_sources] $[st_sources]
  #set so_sources
#endif

// And these are the various source files, extracted out by type.
#define cxx_so_sources $[filter_out %_src.cxx,$[filter %.cxx,$[so_sources]]]
#define cxx_st_sources $[filter_out %_src.cxx,$[filter %.cxx,$[st_sources]]]
#define c_so_sources $[filter %.c,$[so_sources]]
#define c_st_sources $[filter %.c,$[st_sources]]
#define yxx_so_sources $[filter %.yxx,$[so_sources]]
#define yxx_st_sources $[filter %.yxx,$[st_sources]]
#define lxx_so_sources $[filter %.lxx,$[so_sources]]
#define lxx_st_sources $[filter %.lxx,$[st_sources]]

#if $[DO_PCH]
#define pch_header_source $[get_precompiled_header(metalib_target lib_target noinst_lib_target)]

#define st_pch_files $[patsubst %.h,$[st_dir]/%.pch,$[pch_header_source]]
#define st_pch_obj_files $[patsubst %.h,$[st_dir]/%.obj,$[pch_header_source]]

#endif

// This map variable gets us all the various source files from all the
// targets in this directory.  We need it to look up the context in
// which to build a particular source file, since some targets may
// have different requirements (e.g. different local_libs, or
// different USE_this or USE_that) than other targets.
#map all_sources get_sources(metalib_target lib_target noinst_lib_target static_lib_target ss_lib_target bin_target noinst_bin_target test_bin_target)

// We define $[complete_local_libs] as the full set of libraries (from
// within this tree) that we must link a particular target with.  It
// is the transitive closure of our dependent libs: the libraries we
// depend on, plus the libraries *those* libraries depend on, and so
// on.
#defer complete_local_libs $[unique $[closure all_libs,$[active_libs]]]
#defer actual_local_libs $[get_metalibs $[TARGET],$[complete_local_libs]]

// And $[complete_ipath] is the list of directories (from within this
// tree) we should add to our -I list.  It's basically just one for
// each directory named in the $[complete_local_libs], above, plus
// whatever else the user might have explicitly named in
// $[LOCAL_INCS].
#defer complete_ipath $[all_libs $[RELDIR],$[complete_local_libs]] $[RELDIR($[LOCAL_INCS:%=%/])]

// $[target_ipath] is the proper ipath to put on the command line,
// from the context of a particular target.
#defer target_ipath $[TOPDIR] $[sort $[complete_ipath]] $[other_trees:%=%/include] $[get_ipath]

// $[file_ipath] is the ipath from the context of a particular source
// file, given in $[file].  It uses the all_sources map to look up
// the target the source file belongs on, to get the proper context.
#defer file_ipath $[all_sources $[target_ipath],$[file]]

// These are the complete set of extra flags the compiler requires,
// from the context of a particular file, given in $[file].
#defer cflags $[all_sources $[get_cflags] $[CFLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]] 
#defer c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]] 

// These are the same flags, sans the compiler optimizations.
#defer noopt_c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]

// $[complete_lpath] is rather like $[complete_ipath]: the list of
// directories (from within this tree) we should add to our -L list.
#defer complete_lpath $[static_libs $[RELDIR:%=%/$[st_dir]],$[actual_local_libs]] $[dynamic_libs $[RELDIR:%=%/$[so_dir]],$[actual_local_libs]]

// $[lpath] is like $[target_ipath]: it's the list of directories we
// should add to our -L list, from the context of a particular target.
#defer lpath $[sort $[complete_lpath]] $[other_trees:%=%/lib] $[get_lpath]

// And $[libs] is the set of libraries we will link with.
#defer libs $[unique $[actual_local_libs:%=%$[dllext]] $[patsubst %:c,,%:m %,%$[dllext],$[OTHER_LIBS]] $[get_libs]]

// Okay, we're ready.  Start outputting the Makefile now.
#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

// If we are using GNU make, this will automatically enable the
// multiprocessor build mode according to the value in
// NUMBER_OF_PROCESSORS, which should be set by NT.  Maybe this isn't
// a good idea to do all the time, but you can always disable it by
// explicitly unsetting NUMBER_OF_PROCESSORS, or by setting it to 1.
#if $[NUMBER_OF_PROCESSORS]
MAKEFLAGS := -j$[NUMBER_OF_PROCESSORS]
#endif

// The 'all' rule makes all the stuff in the directory except for the
// test_bin_targets.  It doesn't do any installation, however.
#define all_targets \
    Makefile \
    $[if $[dep_sources],$[DEPENDENCY_CACHE_FILENAME]] \
    $[sort $[lib_targets] $[static_lib_targets] $[bin_targets]] \
    $[deferred_objs]
all : $[all_targets]

// The 'test' rule makes all the test_bin_targets.
test : $[test_bin_targets]

// We implement 'clean' simply by removing the odirs, since all of our
// generated output ends up in one or the other of these.  Effective.
// It does assume that the odirs are not '.', however.
clean :
#if $[so_sources]
$[TAB] rm -rf $[so_dir]
#endif
#if $[st_sources]
$[TAB] rm -rf $[st_dir]
#endif
$[TAB] rm -f *.pyc *.pyo  // Also scrub out old generated Python code.


// 'cleanall' is not much more thorough than 'clean': At the moment,
// it also cleans up the bison and flex output, as well as the
// dependency cache file.
cleanall : clean
#if $[yxx_so_sources] $[yxx_st_sources] $[lxx_so_sources] $[lxx_st_sources]
$[TAB] rm -f $[patsubst %.yxx %.lxx,%.cxx,$[yxx_so_sources] $[yxx_st_sources] $[lxx_so_sources] $[lxx_st_sources]]
#endif
#if $[ne $[DEPENDENCY_CACHE_FILENAME],]
$[TAB] rm -f $[DEPENDENCY_CACHE_FILENAME]
#endif

clean-igate :
#forscopes metalib_target lib_target ss_lib_target
  #define igatedb $[get_igatedb]
  #define igateoutput $[get_igateoutput]
  #define igatemscan $[components $[get_igatedb:%=$[RELDIR]/$[so_dir]/%],$[active_component_libs]]
  #define igatemout $[if $[igatemscan],lib$[TARGET]_module.cxx]
  #if $[igatedb]
$[TAB] rm -f $[so_dir]/$[igatedb]
  #endif
  #if $[igateoutput]
$[TAB] rm -f $[so_dir]/$[igateoutput] $[igateoutput:%.cxx=$[so_dir]/%.obj]
  #endif
  #if $[igatemout]
$[TAB] rm -f $[so_dir]/$[igatemout] $[igatemout:%.cxx=$[so_dir]/%.obj]
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
     $[get_igatedb(metalib_target lib_target ss_lib_target):%=$[install_igatedb_dir]/%]

#define install_targets \
     $[sort \
       $[if $[install_lib],$[install_lib_dir]] \
       $[if $[install_bin] $[install_scripts],$[install_bin_dir]] \
       $[if $[install_headers],$[install_headers_dir]] \
       $[if $[install_parser_inc],$[install_parser_inc_dir]] \
       $[if $[install_data],$[install_data_dir]] \
       $[if $[install_config],$[install_config_dir]] \
       $[if $[install_igatedb],$[install_igatedb_dir]] \
     ] \
     $[active_target(metalib_target lib_target static_lib_target ss_lib_target):%=install-lib%] \
     $[active_target(bin_target sed_bin_target):%=install-%] \
     $[installed_files]

install : all $[install_targets]

install-igate : $[sort $[installed_igate_files]]

uninstall : $[active_target(metalib_target lib_target static_lib_target ss_lib_target):%=uninstall-lib%] $[active_target(bin_target):%=uninstall-%]
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

uninstall-igate :
#if $[installed_igate_files]
$[TAB] rm -f $[sort $[installed_igate_files]]
#endif


// We need a rule for each directory we might need to make.  This
// loops through the full set of directories and creates a rule to
// make each one, as needed.
#foreach directory $[sort \
    $[if $[install_lib],$[install_lib_dir]] \
    $[if $[install_bin] $[install_scripts],$[install_bin_dir]] \
    $[if $[install_headers],$[install_headers_dir]] \
    $[if $[install_parser_inc],$[install_parser_inc_dir]] \
    $[if $[install_data],$[install_data_dir]] \
    $[if $[install_config],$[install_config_dir]] \
    $[if $[install_igatedb],$[install_igatedb_dir]] \
    ]
$[directory] :
$[TAB] @test -d $[directory] || echo mkdir -p $[directory]
$[TAB] @test -d $[directory] || mkdir -p $[directory]
#end directory

// We need to make the .obj files depend on the $[so_dir] and
// $[st_dir] directories, to guarantee that the directories are built
// before the .obj files are generated, but we cannot depend on the
// directories directly or we get screwed up by the modification
// times.  So we put this phony timestamp file in each directory.
#foreach directory $[sort \
    $[if $[so_sources],$[so_dir]] \
    $[if $[st_sources],$[st_dir]] \
    ]
$[directory]/stamp :
$[TAB] @test -d $[directory] || echo mkdir -p $[directory]
$[TAB] @test -d $[directory] || mkdir -p $[directory]
$[TAB] @touch $[directory]/stamp
#end directory


// Now it's time to start generating the rules to make our actual
// targets.


/////////////////////////////////////////////////////////////////////
// First, the dynamic libraries.  Each lib_target and metalib_target
// is a dynamic library.
/////////////////////////////////////////////////////////////////////

#forscopes metalib_target lib_target

// In Windows, we don't actually build all the libraries.  In
// particular, we don't build any libraries that are listed on a
// metalib.  Is this one such library?
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
#define igatemscan $[components $[get_igatedb:%=$[RELDIR]/$[so_dir]/%],$[active_component_libs]]
#define igatemout $[if $[igatemscan],lib$[TARGET]_module.cxx]

#if $[build_it]
  // Now output the rule to actually link the library from all of its
  // various .obj files.

  #define sources \
   $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[so_dir]/%.obj,%,,$[get_sources] $[igateoutput] $[igatemout]]] \
   $[components $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[RELDIR]/$[so_dir]/%.obj,%,,$[get_sources] $[get_igateoutput] $[get_pch_outputcxx]]],$[active_component_libs]]
   
  #define varname $[subst -,_,lib$[TARGET]_so]
$[varname] = $[sources]
  #define target $[so_dir]/lib$[TARGET]$[dllext].$[dlllib]
  #define sources $($[varname])
  #define flags   $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[CFLAGS_SHARED] $[building_var:%=/D%]
  #define mybasename $[basename $[notdir $[target]]]  
  #define tmpdirname_cyg $[directory]/$[mybasename]
  #define tmpdirname_win $[directory]\$[mybasename]

// not parallel (requires gmake 3.79) because of link.exe conflicts in TMP dir (see audiotraits dir)
#if $[GENERATE_BUILDDATE]
.NOTPARALLEL $[target] : $[sources] $[so_dir]/stamp $[dtool_ver_dir_cyg]/version.rc
// first generate builddate for rc compiler using compiler preprocessor
$[TAB]  mkdir -p $[tmpdirname_cyg]  // this dir-creation-stuff is leftover from trying to resolve parallel link difficulties
        #define VER_RESOURCE "$[tmpdirname_win]\$[mybasename].res"
$[TAB]  cl /nologo /EP "$[dtool_ver_dir]\verdate.cpp"  > "$[tmpdirname_win]\verdate.h"
$[TAB]  rc /n /i"$[tmpdirname_win]" /fo$[VER_RESOURCE] $[filter /D%, $[flags]]  "$[dtool_ver_dir]\version.rc"
  #if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[SHARED_LIB_C++] $[VER_RESOURCE]
  #else  
$[TAB] $[SHARED_LIB_C] $[VER_RESOURCE]
  #endif
#else
.NOTPARALLEL $[target] : $[sources] $[so_dir]/stamp
  #if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[SHARED_LIB_C++]
  #else  
$[TAB] $[SHARED_LIB_C]
  #endif
#endif

#if $[build_dlls]
$[so_dir]/lib$[TARGET]$[dllext].lib : $[so_dir]/lib$[TARGET]$[dllext].dll
#endif
#if $[build_pdbs]
$[so_dir]/lib$[TARGET]$[dllext].pdb : $[so_dir]/lib$[TARGET]$[dllext].dll
#endif

#endif

// Here are the rules to install and uninstall the library and
// everything that goes along with it.
#define installed_files \
    $[if $[build_it], \
      $[if $[build_dlls],$[install_lib_dir]/lib$[TARGET]$[dllext].dll] \
      $[install_lib_dir]/lib$[TARGET]$[dllext].lib \
      $[if $[and $[build_dlls],$[build_pdbs]],$[install_lib_dir]/lib$[TARGET]$[dllext].pdb] \
    ] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%] \
    $[igatedb:%=$[install_igatedb_dir]/%]

install-lib$[TARGET] : $[installed_files]

uninstall-lib$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

#if $[build_dlls]
$[install_lib_dir]/lib$[TARGET]$[dllext].dll : $[so_dir]/lib$[TARGET]$[dllext].dll $[so_dir]/stamp
#define local lib$[TARGET]$[dllext].dll
#define dest $[install_lib_dir]
$[TAB] cp -f $[so_dir]/$[local] $[dest]
#endif

$[install_lib_dir]/lib$[TARGET]$[dllext].lib : $[so_dir]/lib$[TARGET]$[dllext].lib $[so_dir]/stamp
#define local lib$[TARGET]$[dllext].lib
#define dest $[install_lib_dir]
$[TAB] cp -f $[so_dir]/$[local] $[dest]

#if $[and $[build_dlls],$[build_pdbs]]
$[install_lib_dir]/lib$[TARGET]$[dllext].pdb : $[so_dir]/lib$[TARGET]$[dllext].pdb $[so_dir]/stamp
#define local lib$[TARGET]$[dllext].pdb
#define dest $[install_lib_dir]
$[TAB] cp -f $[so_dir]/$[local] $[dest]
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

$[install_igatedb_dir]/$[igatedb] : $[so_dir]/$[igatedb] $[so_dir]/stamp
#define local $[igatedb]
#define dest $[install_igatedb_dir]
$[TAB] cp -f $[so_dir]/$[local] $[dest]

// We have to split this out as a separate rule to properly support
// parallel make.
$[so_dir]/$[igatedb] : $[so_dir]/$[igateoutput]

lib$[TARGET]_igatescan = $[igatescan]
$[so_dir]/$[igateoutput] : $[sort $[patsubst %.h,%.h,%.I,%.I,%.T,%.T,%,,$[dependencies $[igatescan]] $[igatescan:%=./%]]] $[so_dir]/stamp
$[TAB] interrogate -od $[so_dir]/$[igatedb] -oc $[so_dir]/$[igateoutput] $[interrogate_options] -module "$[igatemod]" -library "$[igatelib]" $(lib$[TARGET]_igatescan)

#define target $[igateoutput:%.cxx=$[so_dir]/%.obj]
#define source $[so_dir]/$[igateoutput]
#define ipath . $[target_ipath]
#define flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[CFLAGS_SHARED] $[building_var:%=/D%]
$[target] : $[source] $[so_dir]/stamp
$[TAB] $[COMPILE_C++]
#endif  // $[igatescan]

#if $[igatemout]
// And finally, some additional rules to build the interrogate module
// file into the library, if this is a metalib that includes
// interrogated components.

#define igatelib lib$[TARGET]
#define igatemod $[TARGET]

lib$[TARGET]_igatemscan = $[igatemscan]
#define target $[so_dir]/$[igatemout]
#define sources $(lib$[TARGET]_igatemscan)
$[target] : $[sources] $[so_dir]/stamp
$[TAB] interrogate_module -oc $[target] -module "$[igatemod]" -library "$[igatelib]" -python $[sources]

#define target $[igatemout:%.cxx=$[so_dir]/%.obj]
#define source $[so_dir]/$[igatemout]
#define ipath . $[target_ipath]
#define flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[CFLAGS_SHARED] $[building_var:%=/D%]
$[target] : $[source] $[so_dir]/stamp
$[TAB] $[COMPILE_C++]
#endif  // $[igatescan]

#end metalib_target lib_target




/////////////////////////////////////////////////////////////////////
// Now, the noninstalled dynamic libraries.  These are presumably used
// only within this directory, or at the most within this tree, and
// also presumably will never include interrogate data.  That, plus
// the fact that we don't need to generate install rules, makes it a
// lot simpler.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_lib_target
#define varname $[subst -,_,lib$[TARGET]_so]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[so_dir]/%.obj,%,,$[get_sources]]]
#define target $[so_dir]/lib$[TARGET]$[dllext].$[dlllib]
#define sources $($[varname])
$[target] : $[sources] $[so_dir]/stamp
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[SHARED_LIB_C++]
#else
$[TAB] $[SHARED_LIB_C]
#endif

#if $[build_dlls]
$[so_dir]/lib$[TARGET]$[dllext].lib : $[so_dir]/lib$[TARGET]$[dllext].dll
#endif
#if $[build_pdbs]
$[so_dir]/lib$[TARGET]$[dllext].pdb : $[so_dir]/lib$[TARGET]$[dllext].dll
#endif

#end noinst_lib_target



/////////////////////////////////////////////////////////////////////
// Now the static libraries.  Again, we assume there's no interrogate
// interfaces going on in here, and there's no question of this being
// a metalib, making the rules relatively simple.
/////////////////////////////////////////////////////////////////////

#forscopes static_lib_target ss_lib_target
#define varname $[subst -,_,lib$[TARGET]_a]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[st_dir]/%.obj,%,,$[get_sources]]]
#define target $[st_dir]/lib$[TARGET]$[dllext].lib
#define sources $($[varname])
$[target] : $[sources] $[st_dir]/stamp
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[STATIC_LIB_C++]
#else
$[TAB] $[STATIC_LIB_C]
#endif

#define installed_files \
    $[install_lib_dir]/lib$[TARGET]$[dllext].lib \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-lib$[TARGET] : $[installed_files]

uninstall-lib$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

$[install_lib_dir]/lib$[TARGET]$[dllext].lib : $[st_dir]/lib$[TARGET]$[dllext].lib $[st_dir]/stamp
#define local lib$[TARGET]$[dllext].lib
#define dest $[install_lib_dir]
$[TAB] cp -f $[st_dir]/$[local] $[dest]

#end static_lib_target ss_lib_target



/////////////////////////////////////////////////////////////////////
// The sed_bin_targets are a special bunch.  These are scripts that
// are to be preprocessed with sed before being installed, for
// instance to insert a path or something in an appropriate place.
/////////////////////////////////////////////////////////////////////

#forscopes sed_bin_target
$[TARGET] : $[st_dir]/$[TARGET] $[st_dir]/stamp

#define target $[st_dir]/$[TARGET]
#define source $[SOURCE]
#define script $[COMMAND]
$[target] : $[source] $[st_dir]/stamp
$[TAB] $[SED]
$[TAB] chmod +x $[target]

#define installed_files \
    $[install_bin_dir]/$[TARGET]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

#define local $[TARGET]
#define dest $[install_bin_dir]
$[install_bin_dir]/$[TARGET] : $[st_dir]/$[TARGET] $[st_dir]/stamp
$[TAB] cp -f $[st_dir]/$[local] $[dest]

#end sed_bin_target


/////////////////////////////////////////////////////////////////////
// And now, the bin_targets.  These are normal C++ executables.  No
// interrogate, metalibs, or any such nonsense here.
/////////////////////////////////////////////////////////////////////

#forscopes bin_target
$[TARGET] : $[st_dir]/$[TARGET].exe $[st_dir]/stamp

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[st_dir]/%.obj,%,,$[get_sources]]]
#define target $[st_dir]/$[TARGET].exe
#define sources $($[varname])
#define ld $[get_ld]
$[target] : $[sources] $[st_dir]/stamp
#if $[ld]
  // If there's a custom linker defined for the target, we have to use it.
$[TAB] $[ld] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]   
#else
  // Otherwise, we can use the normal linker.
  #if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[LINK_BIN_C++]
  #else
$[TAB] $[LINK_BIN_C]
  #endif
#endif

#if $[build_pdbs]
$[st_dir]/$[TARGET].pdb : $[st_dir]/$[TARGET].exe $[st_dir]/stamp
#endif

#define installed_files \
    $[install_bin_dir]/$[TARGET].exe \
    $[if $[build_pdbs],$[install_bin_dir]/$[TARGET].pdb] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
$[TAB] rm -f $[sort $[installed_files]]
#endif

$[install_bin_dir]/$[TARGET].exe : $[st_dir]/$[TARGET].exe $[st_dir]/stamp
#define local $[TARGET].exe
#define dest $[install_bin_dir]
$[TAB] cp -f $[st_dir]/$[local] $[dest]

#if $[build_pdbs]
$[install_bin_dir]/$[TARGET].pdb : $[st_dir]/$[TARGET].pdb $[st_dir]/stamp
#define local $[TARGET].pdb
#define dest $[install_bin_dir]
$[TAB] cp -f $[st_dir]/$[local] $[dest]
#endif

#end bin_target



/////////////////////////////////////////////////////////////////////
// The noinst_bin_targets and the test_bin_targets share the property
// of being built (when requested), but having no install rules.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_bin_target test_bin_target
$[TARGET] : $[st_dir]/$[TARGET].exe $[st_dir]/stamp

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[st_dir]/%.obj,%,,$[get_sources]]]
#define target $[st_dir]/$[TARGET].exe
#define sources $($[varname])
$[target] : $[sources] $[st_dir]/stamp
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB] $[LINK_BIN_C++]
#else
$[TAB] $[LINK_BIN_C]
#endif

#end noinst_bin_target test_bin_target




/////////////////////////////////////////////////////////////////////
// Finally, we put in the rules to compile each source file into a .obj
// file.
/////////////////////////////////////////////////////////////////////

// Rules to generate a C++ file from a Bison input file.
#foreach file $[sort $[yxx_so_sources] $[yxx_st_sources]]
#define target $[patsubst %.yxx,%.cxx,$[file]]
#define source $[file]
$[target] : $[source]
$[TAB] $[BISON] $[YFLAGS] -y $[if $[YACC_PREFIX],-d --name-prefix=$[YACC_PREFIX]] $[source]
$[TAB] mv y.tab.c $[target]
$[TAB] mv y.tab.h $[patsubst %.yxx,%.h,$[source]]

#end file

// Rules to generate a C++ file from a Flex input file.
#foreach file $[sort $[lxx_so_sources] $[lxx_st_sources]]
#define target $[patsubst %.lxx,%.cxx,$[file]]
#define source $[file]
$[target] : $[source]
$[TAB] $[FLEX] $[LFLAGS] $[if $[YACC_PREFIX],-P$[YACC_PREFIX]] -olex.yy.c $[source]
#define source lex.yy.c
#define script /#include <unistd.h>/d
$[TAB] $[SED]
$[TAB] rm $[source]

#end file

// Rules to compile ordinary C files that appear on a shared library.
#foreach file $[sort $[c_so_sources]]
#define target $[patsubst %.c,$[so_dir]/%.obj,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[cflags] $[CFLAGS_SHARED] $[all_sources $[building_var:%=/D%],$[file]]
$[target] : $[source] $[dependencies $[source]] $[so_dir]/stamp
$[TAB] $[COMPILE_C]

#end file

// Rules to compile ordinary C files that appear on a static library
// or in an executable.
#foreach file $[sort $[c_st_sources]]
#define target $[patsubst %.c,$[st_dir]/%.obj,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[cflags] $[all_sources $[building_var:%=/D%],$[file]]
$[target] : $[source] $[dependencies $[source]] $[st_pch_files] $[st_dir]/stamp
$[TAB] $[COMPILE_C]

#end file

// Rules to compile C++ files that appear on a shared library.
#foreach file $[sort $[cxx_so_sources]]
#define target $[patsubst %.cxx,$[so_dir]/%.obj,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[c++flags] $[CFLAGS_SHARED] $[all_sources $[building_var:%=/D%],$[file]]
// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target] : $[source] $[dependencies $[file]] $[yxx_so_sources:%.yxx=%.cxx] $[so_dir]/stamp
$[TAB] $[COMPILE_C++]

#end file

// Rules to compile C++ files that appear on a static library or in an
// executable.

//#foreach file $[sort $[filter-out %_headers.cxx, $[cxx_st_sources]]]
#foreach file $[sort $[cxx_st_sources]]
#define target $[patsubst %.cxx,$[st_dir]/%.obj,$[file]]
#define source $[file]
#define ipath $[file_ipath]

#if $[DO_PCH]
// best way to find out if file use pch (and needs /Yu) is to check dependencies
// these must be defined before flags (or could defer them)
#define target_pch $[subst /./,/,$[patsubst %.h,$[st_dir]/%.pch,$[filter %_headers.h, $[dependencies $[file]]]]]
#define target_dirname $[patsubst %_headers.pch,%,$[target_pch]] 
#endif

#define flags $[c++flags] $[all_sources $[building_var:%=/D%],$[file]]

#if $[target_pch]
#define COMPILE_LINE $[COMPILE_C_WITH_PCH]
#else
#define COMPILE_LINE $[COMPILE_C++]
#endif

$[target] : $[source] $[dependencies $[file]] $[yxx_st_sources:%.yxx=%.cxx] $[target_pch] $[st_dir]/stamp
$[TAB] $[COMPILE_LINE]

#end file

#if $[DO_PCH]
// Rules to compile _headers.pch from _header.h in static lib
#foreach file $[pch_header_source]
#define target_pch $[patsubst %.h,$[st_dir]/%.pch,$[file]]
#define target_obj $[patsubst %.h,$[st_dir]/%.obj,$[file]]
#define target $[target_obj]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[c++flags] $[CFLAGS_SHARED] $[all_sources $[building_var:%=/D%],$[file]]
// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target_obj] : $[source] $[dependencies $[file]] $[st_dir]/stamp
$[TAB] $[COMPILE_CXXSTYLE_PCH]

$[target_pch] : $[target_obj]

#end file
#endif

// Rules to compile generated C++ files that appear on a shared library.
#foreach file $[sort $[yxx_so_sources] $[lxx_so_sources]]
#define target $[patsubst %.lxx %.yxx,$[so_dir]/%.obj,$[file]]
#define source $[patsubst %.lxx %.yxx,%.cxx,$[file]]
#define ipath $[file_ipath]
#define flags $[noopt_c++flags] $[CFLAGS_SHARED] $[all_sources $[building_var:%=/D%],$[file]]
// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target] : $[source] $[dependencies $[file]] $[yxx_so_sources:%.yxx=%.cxx] $[so_dir]/stamp
$[TAB] $[COMPILE_C++]

#end file

// Rules to compile generated C++ files that appear on a static
// library or in an executable.
#foreach file $[sort $[yxx_st_sources] $[lxx_st_sources]]
#define target $[patsubst %.lxx %.yxx,$[st_dir]/%.obj,$[file]]
#define source $[patsubst %.lxx %.yxx,%.cxx,$[file]]
#define ipath $[file_ipath]
#define flags $[noopt_c++flags] $[all_sources $[building_var:%=/D%],$[file]]
$[target] : $[source] $[dependencies $[file]] $[yxx_st_sources:%.yxx=%.cxx] $[st_dir]/stamp
$[TAB] $[COMPILE_C++]

#end file

// And now the rules to install the auxiliary files, like headers and
// data files.
#foreach file $[install_scripts]
$[install_bin_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_bin_dir]
$[TAB] cp -f $[local] $[dest]
#end file

#foreach file $[install_headers]
$[install_headers_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_headers_dir]
$[TAB] cp -f $[local] $[dest]
#end file

#foreach file $[install_parser_inc]
$[install_parser_inc_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_parser_inc_dir]
$[TAB] cp -f $[local] $[dest]
#end file

#foreach file $[install_data]
$[install_data_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_data_dir]
$[TAB] cp -f $[local] $[dest]
#end file

#foreach file $[install_config]
$[install_config_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_config_dir]
$[TAB] cp -f $[local] $[dest]
#end file

// Finally, all the special targets.  These are commands that just need
// to be invoked; we don't pretend to know what they are.
#forscopes special_target
$[TARGET] :
$[TAB] $[COMMAND]

#end special_target


// Finally, the rules to freshen the Makefile itself.
Makefile : $[SOURCE_FILENAME]
$[TAB] ppremake

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

#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

all : $[subdirs]
test : $[subdirs:%=test-%]
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

#if $[ne $[CONFIG_HEADER],]
$[install_headers_dir] :
$[TAB] @test -d $[install_headers_dir] || echo mkdir -p $[install_headers_dir]
$[TAB] @test -d $[install_headers_dir] || mkdir -p $[install_headers_dir]

$[install_headers_dir]/$[CONFIG_HEADER] : $[CONFIG_HEADER]
#define local $[CONFIG_HEADER]
#define dest $[install_headers_dir]
$[TAB] cp -f $[local] $[dest]
#endif

#end Makefile

// If there is a file called LocalSetup.pp in the package's top
// directory, then invoke that.  It might contain some further setup
// instructions.
#sinclude $[TOPDIRPREFIX]LocalSetup.gmsvc.pp
#sinclude $[TOPDIRPREFIX]LocalSetup.pp



//////////////////////////////////////////////////////////////////////
#elif $[or $[eq $[DIR_TYPE], models],$[eq $[DIR_TYPE], models_toplevel],$[eq $[DIR_TYPE], models_group]]
//////////////////////////////////////////////////////////////////////

#include $[THISDIRPREFIX]Template.models.pp

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE

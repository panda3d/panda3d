//
// Template.unix.pp
//
// This file defines the set of output files that will be generated to
// support a generic Unix-style build system.  It generates a number
// of shared libraries named libtarget.so for each lib_target, assumes
// object files are named file.o, and makes other Unix-like
// assumptions.
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
// $DTOOL/pptempl/Global.unix.pp
// $DTOOL/pptempl/Depends.pp, once for each Sources.pp file
// Template.unix.pp (this file), once for each Sources.pp file


//////////////////////////////////////////////////////////////////////
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib]]
//////////////////////////////////////////////////////////////////////

// For a source directory, build a single Makefile with rules to build
// each target.

// We need to know the various targets we'll be building.
// $[lib_targets] will be the list of dynamic libraries,
// $[static_lib_targets] the list of static libraries, and
// $[bin_targets] the list of binaries.  $[test_bin_targets] is the
// list of binaries that are to be built only when specifically asked
// for.
#if $[build_directory]
  #define lib_targets $[active_target(metalib_target lib_target ss_lib_target noinst_lib_target):%=$[so_dir]/lib%.so]
  #define static_lib_targets $[active_target(static_lib_target):%=$[st_dir]/lib%.a]
  #define bin_targets $[active_target(bin_target noinst_bin_target sed_bin_target):%=$[st_dir]/%]
  #define test_bin_targets $[active_target(test_bin_target):%=$[st_dir]/%]

  // And these variables will define the various things we need to
  // install.
  #define install_lib $[active_target(metalib_target lib_target ss_lib_target static_lib_target)]
  #define install_bin $[active_target(bin_target)]
  #define install_scripts $[sort $[INSTALL_SCRIPTS(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_SCRIPTS]]
  #define install_headers $[sort $[INSTALL_HEADERS(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_HEADERS]]
  #define install_parser_inc $[sort $[INSTALL_PARSER_INC]]
  #define install_data $[sort $[INSTALL_DATA(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_DATA]]
  #define install_config $[sort $[INSTALL_CONFIG(metalib_target lib_target ss_lib_target static_lib_target bin_target)] $[INSTALL_CONFIG]]
  #define install_igatedb $[sort $[get_igatedb(metalib_target lib_target ss_lib_target)]]

  // $[so_sources] is the set of sources that belong on a shared object,
  // and $[st_sources] is the set of sources that belong on a static
  // object, like a static library or an executable.  We make the
  // distinction because some architectures require a special parameter
  // to the compiler when we're compiling something to be put in a
  // shared object (to make the code relocatable).
  #define so_sources $[get_sources(metalib_target lib_target ss_lib_target noinst_lib_target)]
  #define st_sources $[get_sources(static_lib_target bin_target noinst_bin_target test_bin_target)]
  
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
  
#endif  // $[build_directory]

// And these are the various source files, extracted out by type.
#define cxx_so_sources $[filter_out %_src.cxx,$[filter %.cxx,$[so_sources]]]
#define cxx_st_sources $[filter_out %_src.cxx,$[filter %.cxx,$[st_sources]]]
#define c_so_sources $[filter %.c,$[so_sources]]
#define c_st_sources $[filter %.c,$[st_sources]]
#define yxx_so_sources $[filter %.yxx,$[so_sources]]
#define yxx_st_sources $[filter %.yxx,$[st_sources]]
#define lxx_so_sources $[filter %.lxx,$[so_sources]]
#define lxx_st_sources $[filter %.lxx,$[st_sources]]

// This map variable gets us all the various source files from all the
// targets in this directory.  We need it to look up the context in
// which to build a particular source file, since some targets may
// have different requirements (e.g. different local_libs, or
// different USE_this or USE_that) than other targets.
#map all_sources get_sources(metalib_target lib_target ss_lib_target noinst_lib_target static_lib_target bin_target noinst_bin_target test_bin_target)

// We define $[complete_local_libs] as the full set of libraries (from
// within this tree) that we must link a particular target with.  It
// is the transitive closure of our dependent libs: the libraries we
// depend on, plus the libraries *those* libraries depend on, and so
// on.
#defer complete_local_libs $[unique $[closure all_libs,$[active_libs]]]

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
#defer cflags $[all_sources $[get_cflags] $[CFLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]] $[if $[>= $[OPTIMIZE],2],$[OPTFLAGS]]
#defer c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]] $[if $[>= $[OPTIMIZE],2],$[OPTFLAGS]]

// These are the same flags, sans the compiler optimizations.
#defer noopt_c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]

// $[complete_lpath] is rather like $[complete_ipath]: the list of
// directories (from within this tree) we should add to our -L list.
#defer complete_lpath $[static_libs $[RELDIR:%=%/$[st_dir]],$[complete_local_libs]] $[dynamic_libs $[RELDIR:%=%/$[so_dir]],$[complete_local_libs]]

// $[lpath] is like $[target_ipath]: it's the list of directories we
// should add to our -L list, from the context of a particular target.
#defer lpath $[sort $[complete_lpath]] $[other_trees:%=%/lib] $[get_lpath]

// And $[libs] is the set of libraries we will link with.
#defer libs $[unique $[complete_local_libs] $[patsubst %:m,,%:c %,%,$[OTHER_LIBS]] $[get_libs]]

// This is the set of files we might copy into *.prebuilt, if we have
// bison and flex (or copy from *.prebuilt if we don't have them).
#define bison_prebuilt $[patsubst %.yxx,%.h,$[yxx_so_sources] $[yxx_st_sources]] $[patsubst %.yxx,%.cxx,$[yxx_so_sources] $[yxx_st_sources]] $[patsubst %.lxx,%.cxx,$[lxx_so_sources] $[lxx_st_sources]]

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
    $[if $[so_sources],$[so_dir]] \
    $[if $[st_sources],$[st_dir]] \
    $[sort $[lib_targets] $[static_lib_targets] $[bin_targets]] \
    $[TARGET(special_target)]
all : $[all_targets]

// The 'test' rule makes all the test_bin_targets.
test : $[test_bin_targets]

// We implement 'clean' simply by removing the odirs, since all of our
// generated output ends up in one or the other of these.  Effective.
// It does assume that the odirs are not '.', however.
clean :
#if $[so_sources]
$[TAB]rm -rf $[so_dir]
#endif
#if $[st_sources]
$[TAB]rm -rf $[st_dir]
#endif
$[TAB]rm -f *.pyc *.pyo  // Also scrub out old generated Python code.

// 'cleanall' is not much more thorough than 'clean': At the moment,
// it also cleans up the bison and flex output, as well as the
// dependency cache file.
cleanall : clean
#if $[yxx_so_sources] $[yxx_st_sources] $[lxx_so_sources] $[lxx_st_sources]
$[TAB]rm -f $[patsubst %.yxx %.lxx,%.cxx,$[yxx_so_sources] $[yxx_st_sources] $[lxx_so_sources] $[lxx_st_sources]]
#endif
#if $[ne $[DEPENDENCY_CACHE_FILENAME],]
$[TAB]rm -f $[DEPENDENCY_CACHE_FILENAME]
#endif

clean-igate :
#forscopes metalib_target lib_target ss_lib_target
  #define igatedb $[get_igatedb]
  #define igateoutput $[get_igateoutput]
  #define igatemscan $[components $[get_igatedb:%=$[RELDIR]/$[so_dir]/%],$[active_component_libs]]
  #define igatemout $[if $[igatemscan],lib$[TARGET]_module.cxx]
  #if $[igatedb]
$[TAB]rm -f $[so_dir]/$[igatedb]
  #endif
  #if $[igateoutput]
$[TAB]rm -f $[so_dir]/$[igateoutput] $[igateoutput:%.cxx=$[so_dir]/%.o]
  #endif
  #if $[igatemout]
$[TAB]rm -f $[so_dir]/$[igatemout] $[igatemout:%.cxx=$[so_dir]/%.o]
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
     $[active_target(metalib_target lib_target ss_lib_target static_lib_target):%=install-lib%] \
     $[active_target(bin_target sed_bin_target):%=install-%] \
     $[installed_files]
install : all $[install_targets]

install-igate : $[sort $[installed_igate_files]]

uninstall : $[TARGET(metalib_target lib_target ss_lib_target static_lib_target):%=uninstall-lib%] $[TARGET(bin_target):%=uninstall-%]
#if $[installed_files]
$[TAB]rm -f $[sort $[installed_files]]
#endif

uninstall-igate :
#if $[installed_igate_files]
$[TAB]rm -f $[sort $[installed_igate_files]]
#endif

#if $[HAVE_BISON]
prebuild-bison : $[patsubst %,%.prebuilt,$[bison_prebuilt]]
clean-prebuild-bison : 
$[TAB]rm -f $[sort $[patsubst %,%.prebuilt,$[bison_prebuilt]]]
#endif

// We need a rule for each directory we might need to make.  This
// loops through the full set of directories and creates a rule to
// make each one, as needed.
#foreach directory $[sort \
    $[if $[so_sources],$[so_dir]] \
    $[if $[st_sources],$[st_dir]] \
    $[if $[install_lib],$[install_lib_dir]] \
    $[if $[install_bin] $[install_scripts],$[install_bin_dir]] \
    $[if $[install_headers],$[install_headers_dir]] \
    $[if $[install_parser_inc],$[install_parser_inc_dir]] \
    $[if $[install_data],$[install_data_dir]] \
    $[if $[install_config],$[install_config_dir]] \
    $[if $[install_igatedb],$[install_igatedb_dir]] \
    ]
$[directory] :
$[TAB]@test -d $[directory] || echo mkdir -p $[directory]
$[TAB]@test -d $[directory] || mkdir -p $[directory]
#end directory


// Now it's time to start generating the rules to make our actual
// targets.

// Determine which files will be generated during the interrogate
// pass, and make a special "igate" rule to generate all of them.
#define build_igate
#forscopes metalib_target lib_target ss_lib_target
  #define igatemscan $[components $[get_igatedb],$[active_component_libs]]
  #define igatemout $[if $[igatemscan],lib$[TARGET]_module.cxx]
  #set build_igate $[build_igate] $[get_igatedb:%=$[so_dir]/%] $[igatemout:%=$[so_dir]/%]
#end metalib_target lib_target ss_lib_target
igate : $[sort $[build_igate]]


/////////////////////////////////////////////////////////////////////
// First, the dynamic libraries.  Each lib_target and metalib_target
// is a dynamic library.
/////////////////////////////////////////////////////////////////////

#forscopes metalib_target lib_target ss_lib_target

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

// Now output the rule to actually link the library from all of its
// various .o files.
#define varname $[subst -,_,lib$[TARGET]_so]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[so_dir]/%.o,%,,$[get_sources] $[igateoutput] $[igatemout]]]
#define target $[so_dir]/lib$[TARGET].so
#define sources $($[varname])
$[target] : $[sources]
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB]$[SHARED_LIB_C++]
#else
$[TAB]$[SHARED_LIB_C]
#endif

// Here are the rules to install and uninstall the library and
// everything that goes along with it.
#define installed_files \
    $[install_lib_dir]/lib$[TARGET].so \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%] \
    $[igatedb:%=$[install_igatedb_dir]/%]

install-lib$[TARGET] : $[installed_files]

uninstall-lib$[TARGET] :
#if $[installed_files]
$[TAB]rm -f $[sort $[installed_files]]
#endif

$[install_lib_dir]/lib$[TARGET].so : $[so_dir]/lib$[TARGET].so
#define local lib$[TARGET].so
#define dest $[install_lib_dir]
$[TAB]cd ./$[so_dir] && $[INSTALL_PROG]

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

$[install_igatedb_dir]/$[igatedb] : $[so_dir]/$[igatedb]
#define local $[igatedb]
#define dest $[install_igatedb_dir]
$[TAB]cd ./$[so_dir] && $[INSTALL]

// We have to split this out as a separate rule to properly support
// parallel make.
$[so_dir]/$[igatedb] : $[so_dir]/$[igateoutput]

lib$[TARGET]_igatescan = $[igatescan]
$[so_dir]/$[igateoutput] : $[sort $[patsubst %.h,%.h,%.I,%.I,%.T,%.T,%,,$[dependencies $[igatescan]] $[igatescan:%=./%]]]
$[TAB]$[INTERROGATE] -od $[so_dir]/$[igatedb] -oc $[so_dir]/$[igateoutput] $[interrogate_options] -module "$[igatemod]" -library "$[igatelib]" $(lib$[TARGET]_igatescan)

#define target $[igateoutput:%.cxx=$[so_dir]/%.o]
#define source $[so_dir]/$[igateoutput]
#define ipath . $[target_ipath]
#define flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[if $[>= $[OPTIMIZE],2],$[OPTFLAGS]] $[CFLAGS_SHARED]
$[target] : $[source]
$[TAB]$[COMPILE_C++]
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
$[target] : $[sources]
$[TAB]$[INTERROGATE_MODULE] -oc $[target] -module "$[igatemod]" -library "$[igatelib]" $[interrogate_module_options] $[sources]

#define target $[igatemout:%.cxx=$[so_dir]/%.o]
#define source $[so_dir]/$[igatemout]
#define ipath . $[target_ipath]
#define flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[if $[>= $[OPTIMIZE],2],$[OPTFLAGS]] $[CFLAGS_SHARED]
$[target] : $[source]
$[TAB]$[COMPILE_C++]
#endif  // $[igatescan]

#end metalib_target lib_target ss_lib_target




/////////////////////////////////////////////////////////////////////
// Now, the noninstalled dynamic libraries.  These are presumably used
// only within this directory, or at the most within this tree, and
// also presumably will never include interrogate data.  That, plus
// the fact that we don't need to generate install rules, makes it a
// lot simpler.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_lib_target
#define varname $[subst -,_,lib$[TARGET]_so]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[so_dir]/%.o,%,,$[get_sources]]]
#define target $[so_dir]/lib$[TARGET].so
#define sources $($[varname])
$[target] : $[sources]
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB]$[SHARED_LIB_C++]
#else
$[TAB]$[SHARED_LIB_C]
#endif

#end noinst_lib_target



/////////////////////////////////////////////////////////////////////
// Now the static libraries.  Again, we assume there's no interrogate
// interfaces going on in here, and there's no question of this being
// a metalib, making the rules relatively simple.
/////////////////////////////////////////////////////////////////////

#forscopes static_lib_target
#define varname $[subst -,_,lib$[TARGET]_a]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[st_dir]/%.o,%,,$[get_sources]]]
#define target $[st_dir]/lib$[TARGET].a
#define sources $($[varname])
$[target] : $[sources]
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB]$[STATIC_LIB_C++]
#else
$[TAB]$[STATIC_LIB_C]
#endif
#if $[RANLIB]
$[TAB]$[RANLIB]
#endif

#define installed_files \
    $[install_lib_dir]/lib$[TARGET].a \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-lib$[TARGET] : $[installed_files]

uninstall-lib$[TARGET] :
#if $[installed_files]
$[TAB]rm -f $[sort $[installed_files]]
#endif

$[install_lib_dir]/lib$[TARGET].a : $[st_dir]/lib$[TARGET].a
#define local lib$[TARGET].a
#define dest $[install_lib_dir]
$[TAB]cd ./$[st_dir] && $[INSTALL]

#end static_lib_target



/////////////////////////////////////////////////////////////////////
// The sed_bin_targets are a special bunch.  These are scripts that
// are to be preprocessed with sed before being installed, for
// instance to insert a path or something in an appropriate place.
/////////////////////////////////////////////////////////////////////

#forscopes sed_bin_target
$[TARGET] : $[st_dir]/$[TARGET]

#define target $[st_dir]/$[TARGET]
#define source $[SOURCE]
#define script $[COMMAND]
$[target] : $[source]
$[TAB]$[SED]
$[TAB]chmod +x $[target]

#define installed_files \
    $[install_bin_dir]/$[TARGET]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
$[TAB]rm -f $[sort $[installed_files]]
#endif

$[install_bin_dir]/$[TARGET] : $[st_dir]/$[TARGET]
#define local $[TARGET]
#define dest $[install_bin_dir]
$[TAB]cd ./$[st_dir] && $[INSTALL_PROG]

#end sed_bin_target


/////////////////////////////////////////////////////////////////////
// And now, the bin_targets.  These are normal C++ executables.  No
// interrogate, metalibs, or any such nonsense here.
/////////////////////////////////////////////////////////////////////

#forscopes bin_target
$[TARGET] : $[st_dir]/$[TARGET]

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[st_dir]/%.o,%,,$[get_sources]]]
#define target $[st_dir]/$[TARGET]
#define sources $($[varname])
#define ld $[get_ld]
$[target] : $[sources]
#if $[ld]
  // If there's a custom linker defined for the target, we have to use it.
$[TAB]$[ld] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]$[TAB]
#else
  // Otherwise, we can use the normal linker.
  #if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB]$[LINK_BIN_C++]
  #else
$[TAB]$[LINK_BIN_C]
  #endif
#endif

#define installed_files \
    $[install_bin_dir]/$[TARGET] \
    $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
    $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
    $[INSTALL_DATA:%=$[install_data_dir]/%] \
    $[INSTALL_CONFIG:%=$[install_config_dir]/%]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
$[TAB]rm -f $[sort $[installed_files]]
#endif

$[install_bin_dir]/$[TARGET] : $[st_dir]/$[TARGET]
#define local $[TARGET]
#define dest $[install_bin_dir]
$[TAB]cd ./$[st_dir] && $[INSTALL_PROG]

#end bin_target



/////////////////////////////////////////////////////////////////////
// The noinst_bin_targets and the test_bin_targets share the property
// of being built (when requested), but having no install rules.
/////////////////////////////////////////////////////////////////////

#forscopes noinst_bin_target test_bin_target
$[TARGET] : $[st_dir]/$[TARGET]

#define varname $[subst -,_,bin_$[TARGET]]
$[varname] = $[unique $[patsubst %_src.cxx,,%.cxx %.c %.yxx %.lxx,$[st_dir]/%.o,%,,$[get_sources]]]
#define target $[st_dir]/$[TARGET]
#define sources $($[varname])
$[target] : $[sources]
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
$[TAB]$[LINK_BIN_C++]
#else
$[TAB]$[LINK_BIN_C]
#endif

#end noinst_bin_target test_bin_target




/////////////////////////////////////////////////////////////////////
// Finally, we put in the rules to compile each source file into a .o
// file.
/////////////////////////////////////////////////////////////////////

// Rules to generate a C++ file from a Bison input file.
#foreach file $[sort $[yxx_so_sources] $[yxx_st_sources]]
#define target $[patsubst %.yxx,%.cxx,$[file]]
#define target_header $[patsubst %.yxx,%.h,$[file]]
#if $[HAVE_BISON]
#define source $[file]
$[target] : $[source]
$[TAB]$[BISON] -y $[YFLAGS] $[if $[YACC_PREFIX],-d --name-prefix=$[YACC_PREFIX]] $[source]
$[TAB]mv y.tab.c $[target]
$[TAB]mv y.tab.h $[target_header]
$[target_header] : $[target]
$[target].prebuilt : $[target]
$[TAB]cp $[target] $[target].prebuilt
$[target_header].prebuilt : $[target_header]
$[TAB]cp $[target_header] $[target_header].prebuilt
#else // HAVE_BISON
#define source $[target].prebuilt
$[target] : $[source]
$[TAB]cp $[source] $[target]
#define source $[target_header].prebuilt
$[target_header] : $[source]
$[TAB]cp $[source] $[target_header]
#endif // HAVE_BISON

#end file

// Rules to generate a C++ file from a Flex input file.
#foreach file $[sort $[lxx_so_sources] $[lxx_st_sources]]
#define target $[patsubst %.lxx,%.cxx,$[file]]
#if $[HAVE_BISON]
#define source $[file]
$[target] : $[source]
$[TAB]$[FLEX] $[LFLAGS] $[if $[YACC_PREFIX],-P$[YACC_PREFIX]] -olex.yy.c $[source]
#define source lex.yy.c
#define script /#include <unistd.h>/d
$[TAB]$[SED]
$[TAB]rm $[source]
$[target].prebuilt : $[target]
$[TAB]cp $[target] $[target].prebuilt
#else // HAVE_BISON
#define source $[target].prebuilt
$[target] : $[source]
$[TAB]cp $[source] $[target]
#endif // HAVE_BISON

#end file


// Rules to compile ordinary C files that appear on a shared library.
#foreach file $[sort $[c_so_sources]]
#define target $[patsubst %.c,$[so_dir]/%.o,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[cflags] $[CFLAGS_SHARED]
$[target] : $[source] $[dependencies $[source]]
$[TAB]$[COMPILE_C]

#end file

// Rules to compile ordinary C files that appear on a static library
// or in an executable.
#foreach file $[sort $[c_st_sources]]
#define target $[patsubst %.c,$[st_dir]/%.o,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[cflags]
$[target] : $[source] $[dependencies $[source]]
$[TAB]$[COMPILE_C]

#end file

// Rules to compile C++ files that appear on a shared library.
#foreach file $[sort $[cxx_so_sources]]
#define target $[patsubst %.cxx,$[so_dir]/%.o,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[c++flags] $[CFLAGS_SHARED]
// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target] : $[source] $[dependencies $[file]] $[yxx_so_sources:%.yxx=%.h]
$[TAB]$[COMPILE_C++]

#end file

// Rules to compile C++ files that appear on a static library or in an
// executable.
#foreach file $[sort $[cxx_st_sources]]
#define target $[patsubst %.cxx,$[st_dir]/%.o,$[file]]
#define source $[file]
#define ipath $[file_ipath]
#define flags $[c++flags]
$[target] : $[source] $[dependencies $[file]] $[yxx_st_sources:%.yxx=%.h]
$[TAB]$[COMPILE_C++]

#end file

// Rules to compile generated C++ files that appear on a shared library.
#foreach file $[sort $[yxx_so_sources] $[lxx_so_sources]]
#define target $[patsubst %.lxx %.yxx,$[so_dir]/%.o,$[file]]
#define source $[patsubst %.lxx %.yxx,%.cxx,$[file]]
#define ipath $[file_ipath]
#define flags $[noopt_c++flags] $[CFLAGS_SHARED]
// Yacc must run before some files can be compiled, so all files
// depend on yacc having run.
$[target] : $[source] $[dependencies $[file]] $[yxx_so_sources:%.yxx=%.h]
$[TAB]$[COMPILE_C++]

#end file

// Rules to compile generated C++ files that appear on a static
// library or in an executable.
#foreach file $[sort $[yxx_st_sources] $[lxx_st_sources]]
#define target $[patsubst %.lxx %.yxx,$[st_dir]/%.o,$[file]]
#define source $[patsubst %.lxx %.yxx,%.cxx,$[file]]
#define ipath $[file_ipath]
#define flags $[noopt_c++flags]
$[target] : $[source] $[dependencies $[file]] $[yxx_st_sources:%.yxx=%.h]
$[TAB]$[COMPILE_C++]

#end file

// And now the rules to install the auxiliary files, like headers and
// data files.
#foreach file $[install_scripts]
$[install_bin_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_bin_dir]
$[TAB]$[INSTALL_PROG]
#end file

#foreach file $[install_headers]
$[install_headers_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_headers_dir]
$[TAB]$[INSTALL]
#end file

#foreach file $[install_parser_inc]
$[install_parser_inc_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_parser_inc_dir]
$[TAB]$[INSTALL]
#end file

#foreach file $[install_data]
$[install_data_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_data_dir]
$[TAB]$[INSTALL]
#end file

#foreach file $[install_config]
$[install_config_dir]/$[file] : $[file]
#define local $[file]
#define dest $[install_config_dir]
$[TAB]$[INSTALL]
#end file

// Finally, all the special targets.  These are commands that just need
// to be invoked; we don't pretend to know what they are.
#forscopes special_target
$[TARGET] :
$[TAB]$[COMMAND]

#end special_target


// Finally, the rules to freshen the Makefile itself.
Makefile : $[SOURCE_FILENAME]
$[TAB]ppremake

#if $[and $[DEPENDENCY_CACHE_FILENAME],$[dep_sources]]
$[DEPENDENCY_CACHE_FILENAME] : $[dep_sources]
$[TAB]@ppremake -D $[DEPENDENCY_CACHE_FILENAME]
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
igate : $[subdirs:%=igate-%]
clean : $[subdirs:%=clean-%]
clean-igate : $[subdirs:%=clean-igate-%]
cleanall : $[subdirs:%=cleanall-%]
install : $[if $[CONFIG_HEADER],$[install_headers_dir] $[install_headers_dir]/$[CONFIG_HEADER]] $[subdirs:%=install-%]
install-igate : $[subdirs:%=install-igate-%]
uninstall : $[subdirs:%=uninstall-%]
#if $[CONFIG_HEADER]
$[TAB]rm -f $[install_headers_dir]/$[CONFIG_HEADER]
#endif
uninstall-igate : $[subdirs:%=uninstall-igate-%]

#if $[HAVE_BISON]
prebuild-bison : $[subdirs:%=prebuild-bison-%]
clean-prebuild-bison : $[subdirs:%=clean-prebuild-bison-%]
#endif

// Somehow, something in the cttools confuses some shells, so that
// when we are attached, 'cd foo' doesn't work, but 'cd ./foo' does.
// Weird.  We get around this by putting a ./ in front of each cd
// target below.

#formap dirname subdirs
#define depends 
$[dirname] : $[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]
$[TAB]cd ./$[PATH] && $(MAKE) all
#end dirname

#formap dirname subdirs
test-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) test
#end dirname

#formap dirname subdirs
igate-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) igate
#end dirname

#formap dirname subdirs
clean-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) clean
#end dirname

#formap dirname subdirs
clean-igate-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) clean-igate
#end dirname

#formap dirname subdirs
cleanall-$[dirname] : $[patsubst %,cleanall-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]
$[TAB]cd ./$[PATH] && $(MAKE) cleanall
#end dirname

#formap dirname subdirs
install-$[dirname] : $[patsubst %,install-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]
$[TAB]cd ./$[PATH] && $(MAKE) install
#end dirname

#formap dirname subdirs
install-igate-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) install-igate
#end dirname

#formap dirname subdirs
uninstall-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) uninstall
#end dirname

#formap dirname subdirs
uninstall-igate-$[dirname] :
$[TAB]cd ./$[PATH] && $(MAKE) uninstall-igate
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
$[TAB]@test -d $[install_headers_dir] || echo mkdir -p $[install_headers_dir]
$[TAB]@test -d $[install_headers_dir] || mkdir -p $[install_headers_dir]

$[install_headers_dir]/$[CONFIG_HEADER] : $[CONFIG_HEADER]
#define local $[CONFIG_HEADER]
#define dest $[install_headers_dir]
$[TAB]$[INSTALL]
#endif

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

//
// Template.unix.pp
//
// This file defines the set of output files that will be generated to
// support a generic Unix-style build system.  It generates a number
// of shared libraries named libtarget.so for each lib_target, assumes
// object files are named file.o, and makes other Unix-like
// assumptions.
//

//////////////////////////////////////////////////////////////////////
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib]]
//////////////////////////////////////////////////////////////////////

// For a source directory, build a single Makefile with rules to build
// each target.

#define lib_targets $[TARGET(metalib_target lib_target noinst_lib_target):%=$[so_dir]/lib%.so]
#define static_lib_targets $[TARGET(static_lib_target):%=$[st_dir]/lib%.a]
#define bin_targets $[TARGET(bin_target noinst_bin_target sed_bin_target):%=$[st_dir]/%]
#define test_bin_targets $[TARGET(test_bin_target):%=$[st_dir]/%]

#define install_lib $[TARGET(metalib_target lib_target static_lib_target)]
#define install_bin $[TARGET(bin_target)]
#define install_scripts $[TARGET(sed_bin_target)] $[sort $[INSTALL_SCRIPTS(metalib_target lib_target static_lib_target bin_target)] $[INSTALL_SCRIPTS]]
#define install_headers $[sort $[INSTALL_HEADERS(metalib_target lib_target static_lib_target bin_target)] $[INSTALL_HEADERS]]
#define install_parser_inc $[sort $[INSTALL_PARSER_INC]]
#define install_data $[sort $[INSTALL_DATA(metalib_target lib_target static_lib_target bin_target)] $[INSTALL_DATA]]
#define install_config $[sort $[INSTALL_CONFIG(metalib_target lib_target static_lib_target bin_target)] $[INSTALL_CONFIG]]
#define install_igatedb $[sort $[get_igatedb(metalib_target lib_target)]]

#define so_sources $[get_sources(metalib_target lib_target noinst_lib_target)]
#define st_sources $[get_sources(static_lib_target bin_target noinst_bin_target test_bin_target)]

// These are the source files that our dependency cache file will
// depend on.  If it's an empty list, we won't bother writing rules to
// freshen the cache file.
#define dep_sources $[sort $[filter %.c %.cxx %.yxx %.lxx %.h %.I,$[so_sources] $[st_sources]]]

#if $[eq $[so_dir],$[st_dir]]
  // If the static and shared directories are the same, we have to use the
  // same rules to build both shared and static targets.
  #set st_sources $[so_sources] $[st_sources]
  #set so_sources
#endif

#define cxx_so_sources $[filter %.cxx,$[so_sources]]
#define cxx_st_sources $[filter %.cxx,$[st_sources]]
#define c_so_sources $[filter %.c,$[so_sources]]
#define c_st_sources $[filter %.c,$[st_sources]]
#define yxx_so_sources $[filter %.yxx,$[so_sources]]
#define yxx_st_sources $[filter %.yxx,$[st_sources]]
#define lxx_so_sources $[filter %.lxx,$[so_sources]]
#define lxx_st_sources $[filter %.lxx,$[st_sources]]

#map all_sources get_sources(metalib_target lib_target noinst_lib_target static_lib_target bin_target noinst_bin_target test_bin_target)

#defer complete_local_libs $[sort $[closure all_libs,$[active_libs]]]
#defer complete_ipath $[all_libs $[RELDIR],$[complete_local_libs]] $[RELDIR($[LOCAL_INCS:%=%/])]
#defer file_ipath $[other_trees:%=%/include] $[TOPDIR] $[sort $[all_sources $[complete_ipath],$[file]]] $[all_sources $[get_ipath],$[file]]
#defer target_ipath $[other_trees:%=%/include] $[TOPDIR] $[sort $[complete_ipath]] $[get_ipath]

#defer cflags $[all_sources $[get_cflags] $[CFLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]
#defer c++flags $[all_sources $[get_cflags] $[C++FLAGS],$[file]] $[CFLAGS_OPT$[OPTIMIZE]]

#defer target_lpath $[other_trees:%=%/lib] $[sort $[static_libs $[RELDIR:%=%/$[st_dir]],$[complete_local_libs]] $[dynamic_libs $[RELDIR:%=%/$[so_dir]],$[complete_local_libs]]]
#defer lpath $[target_lpath] $[get_lpath]
#defer libs $[unique $[complete_local_libs] $[patsubst %:m,,%:c %,%,$[OTHER_LIBS]] $[get_libs]]

#output Makefile
#format makefile
#### Generated automatically by $[PPREMAKE] $[PPREMAKE_VERSION] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#define all_targets \
    Makefile \
    $[if $[dep_sources],$[DEPENDENCY_CACHE_FILENAME]] \
    $[if $[so_sources],$[so_dir]] \
    $[if $[st_sources],$[st_dir]] \
    $[sort $[lib_targets] $[static_lib_targets] $[bin_targets]]
all : $[all_targets]

test : $[test_bin_targets]

clean :
#if $[so_sources]
	rm -rf $[so_dir]
#endif
#if $[st_sources]
	rm -rf $[st_dir]
#endif

cleanall : clean
#if $[yxx_so_sources] $[yxx_st_sources] $[lxx_so_sources] $[lxx_st_sources]
	rm -f $[patsubst %.yxx %.lxx,%.cxx,$[yxx_so_sources] $[yxx_st_sources] $[lxx_so_sources] $[lxx_st_sources]]
#endif
#if $[ne $[DEPENDENCY_CACHE_FILENAME],]
	rm -f $[DEPENDENCY_CACHE_FILENAME]
#endif

#define installed_files \
     $[INSTALL_SCRIPTS:%=$[install_bin_dir]/%] \
     $[INSTALL_HEADERS:%=$[install_headers_dir]/%] \
     $[INSTALL_PARSER_INC:%=$[install_parser_inc_dir]/%] \
     $[INSTALL_DATA:%=$[install_data_dir]/%] \
     $[INSTALL_CONFIG:%=$[install_config_dir]/%]

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
     $[TARGET(metalib_target lib_target static_lib_target):%=install-lib%] $[TARGET(bin_target):%=install-%] \
     $[installed_files]
install : all $[install_targets]

uninstall : $[TARGET(metalib_target lib_target static_lib_target):%=uninstall-lib%] $[TARGET(bin_target):%=uninstall-%]
#if $[installed_files]
	rm -f $[sort $[installed_files]]
#endif


// Define rules to create each install directory when needed.
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
	@test -d $[directory] || echo mkdir -p $[directory]
	@test -d $[directory] || mkdir -p $[directory]
#end directory

#forscopes metalib_target lib_target
#define igatescan $[get_igatescan]
#define igateoutput $[if $[igatescan],lib$[TARGET]_igate.cxx]
#define igatedb $[get_igatedb]

// Should we build a metalib module for all the interrogated libraries
// that are included on this metalib?
#define igatemscan $[components $[get_igatedb:%=$[RELDIR]/$[so_dir]/%],$[active_component_libs]]
#define igatemout $[if $[igatemscan],lib$[TARGET]_module.cxx]

lib_$[TARGET]_so = $[unique $[patsubst %.cxx %.c %.yxx %.lxx,$[so_dir]/%.o,%,,$[get_sources] $[igateoutput] $[igatemout]]]
$[so_dir]/lib$[TARGET].so : $(lib_$[TARGET]_so)
#define target $@
#define sources $(lib_$[TARGET]_so)
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
	$[SHARED_LIB_C++]
#else
	$[SHARED_LIB_C]
#endif

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
	rm -f $[sort $[installed_files]]
#endif

$[install_lib_dir]/lib$[TARGET].so : $[so_dir]/lib$[TARGET].so
#define local $<
#define dest $[install_lib_dir]
	$[INSTALL]

#if $[igatescan]
// Now, some additional rules to build the interrogate file into the
// library, if requested.

// The library name is based on this library.
#define igatelib lib$[TARGET]
// The module name comes from the metalib that includes this library.
#define igatemod $[module $[TARGET],$[TARGET]]
#if $[eq $[igatemod],]
  // Unless no metalib includes this library.
  #define igatemod $[TARGET]
#endif

$[install_igatedb_dir]/$[igatedb] : $[so_dir]/$[igatedb]
#define local $<
#define dest $[install_igatedb_dir]
	$[INSTALL]

lib$[TARGET]_igatescan = $[igatescan]
$[so_dir]/$[igatedb] $[so_dir]/$[igateoutput] : $[filter-out .c .cxx,$[igatescan]]
	interrogate -od $[so_dir]/$[igatedb] -oc $[so_dir]/$[igateoutput] -DCPPPARSER -D__cplusplus $[SYSTEM_IGATE_FLAGS] -S$[install_parser_inc_dir] $[target_ipath:%=-I%] $[filter -D%,$[get_cflags] $[C++FLAGS]] -module "$[igatemod]" -library "$[igatelib]" -fnames -string -refcount -assert -promiscuous -python $(lib$[TARGET]_igatescan)

$[igateoutput:%.cxx=$[so_dir]/%.o] : $[so_dir]/$[igateoutput]
#define target $@
#define source $<
#define ipath . $[target_ipath]
#define flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[CFLAGS_SHARED]
	$[COMPILE_C++]
#endif

#if $[igatemout]
// And finally, some additional rules to build the interrogate module
// file into the library, if this is a metalib that includes
// interrogated components.

#define igatelib lib$[TARGET]
#define igatemod $[TARGET]

lib$[TARGET]_igatemscan = $[igatemscan]
$[so_dir]/$[igatemout] : $(lib$[TARGET]_igatemscan)
	interrogate_module -oc $@ -module "$[igatemod]" -library "$[igatelib]" -python $(lib$[TARGET]_igatemscan)

$[igatemout:%.cxx=$[so_dir]/%.o] : $[so_dir]/$[igatemout]
#define target $@
#define source $<
#define ipath . $[target_ipath]
#define flags $[get_cflags] $[C++FLAGS] $[CFLAGS_OPT$[OPTIMIZE]] $[CFLAGS_SHARED]
	$[COMPILE_C++]
#endif

#end metalib_target lib_target



#forscopes noinst_lib_target
lib_$[TARGET]_so = $[unique $[patsubst %.cxx %.c %.yxx %.lxx,$[so_dir]/%.o,%,,$[get_sources]]]
$[so_dir]/lib$[TARGET].so : $(lib_$[TARGET]_so)
#define target $@
#define sources $(lib_$[TARGET]_so)
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
	$[SHARED_LIB_C++]
#else
	$[SHARED_LIB_C]
#endif

#end noinst_lib_target



#forscopes static_lib_target
lib_$[TARGET]_a = $[unique $[patsubst %.cxx %.c %.yxx %.lxx,$[st_dir]/%.o,%,,$[get_sources]]]
$[st_dir]/lib$[TARGET].a : $(lib_$[TARGET]_a)
#define target $@
#define sources $(lib_$[TARGET]_a)
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
	$[STATIC_LIB_C++]
#else
	$[STATIC_LIB_C]
#endif
#if $[RANLIB]
	$[RANLIB]
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
	rm -f $[sort $[installed_files]]
#endif

$[install_lib_dir]/lib$[TARGET].a : $[st_dir]/lib$[TARGET].a
#define local $<
#define dest $[install_lib_dir]
	$[INSTALL]

#end static_lib_target


#forscopes sed_bin_target
// The sed_bin_target is a special target: it defines a file that is to
// be processed with sed to produce an executable script.  Pretty 
// Unix-specific, so we'd better not have too many of these.
$[st_dir]/$[TARGET] : $[SOURCE]
	$[SED] $[COMMAND] $^ >$@
	chmod +x $@

#define installed_files \
    $[install_bin_dir]/$[TARGET]

install-$[TARGET] : $[installed_files]

uninstall-$[TARGET] :
#if $[installed_files]
	rm -f $[sort $[installed_files]]
#endif

$[install_bin_dir]/$[TARGET] : $[st_dir]/$[TARGET]
#define local $<
#define dest $[install_bin_dir]
	$[INSTALL_PROG]

#end sed_bin_target


#forscopes bin_target
bin_$[TARGET] = $[unique $[patsubst %.cxx %.c %.yxx %.lxx,$[st_dir]/%.o,%,,$[get_sources]]]
$[st_dir]/$[TARGET] : $(bin_$[TARGET])
#define target $@
#define sources $(bin_$[TARGET])
#define ld $[get_ld]
#if $[ld]
  // If there's a custom linker defined for the target, we have to use it.
	$[ld] -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%]	
#else
  // Otherwise, we can use the normal linker.
  #if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
	$[LINK_BIN_C++]
  #else
	$[LINK_BIN_C]
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
	rm -f $[sort $[installed_files]]
#endif

$[install_bin_dir]/$[TARGET] : $[st_dir]/$[TARGET]
#define local $<
#define dest $[install_bin_dir]
	$[INSTALL_PROG]

#end bin_target



#forscopes noinst_bin_target test_bin_target
bin_$[TARGET] = $[unique $[patsubst %.cxx %.c %.yxx %.lxx,$[st_dir]/%.o,%,,$[get_sources]]]
$[st_dir]/$[TARGET] : $(bin_$[TARGET])
#define target $@
#define sources $(bin_$[TARGET])
#if $[filter %.cxx %.yxx %.lxx,$[get_sources]]
	$[LINK_BIN_C++]
#else
	$[LINK_BIN_C]
#endif

#end noinst_bin_target test_bin_target



#foreach file $[sort $[yxx_so_sources] $[yxx_st_sources]]
$[patsubst %.yxx,%.cxx,$[file]] : $[file]
	$[BISON] -y $[if $[YACC_PREFIX],-d --name-prefix=$[YACC_PREFIX]] $<
	mv y.tab.c $@
	mv y.tab.h $[patsubst %.yxx,%.h,$[file]]

#end file
#foreach file $[sort $[lxx_so_sources] $[lxx_st_sources]]
$[patsubst %.lxx,%.cxx,$[file]] : $[file]
	$[FLEX] $[if $[YACC_PREFIX],-P$[YACC_PREFIX]] -olex.yy.c $<
	$[SED] '/#include <unistd.h>/d' lex.yy.c > $@
	rm lex.yy.c

#end file
#foreach file $[sort $[c_so_sources]]
$[patsubst %.c,$[so_dir]/%.o,$[file]] : $[file] $[dependencies $[file]]
#define target $@
#define source $<
#define ipath $[file_ipath]
#define flags $[cflags] $[CFLAGS_SHARED]
	$[COMPILE_C]

#end file
#foreach file $[sort $[c_st_sources]]
$[patsubst %.c,$[st_dir]/%.o,$[file]] : $[file] $[dependencies $[file]]
#define target $@
#define source $<
#define ipath $[file_ipath]
#define flags $[cflags]
	$[COMPILE_C]

#end file
#foreach file $[sort $[cxx_so_sources] $[yxx_so_sources] $[lxx_so_sources]]
$[patsubst %.cxx %.lxx %.yxx,$[so_dir]/%.o,$[file]] : $[patsubst %.cxx %.lxx %.yxx,%.cxx,$[file]] $[dependencies $[file]]
#define target $@
#define source $<
#define ipath $[file_ipath]
#define flags $[c++flags] $[CFLAGS_SHARED]
	$[COMPILE_C++]

#end file
#foreach file $[sort $[cxx_st_sources] $[yxx_st_sources] $[lxx_st_sources]]
$[patsubst %.cxx %.lxx %.yxx,$[st_dir]/%.o,$[file]] : $[patsubst %.cxx %.lxx %.yxx,%.cxx,$[file]] $[dependencies $[file]]
#define target $@
#define source $<
#define ipath $[file_ipath]
#define flags $[c++flags]
	$[COMPILE_C++]

#end file

// And now the rules to install the auxiliary files, like headers and
// data files.
#foreach file $[install_scripts]
$[install_bin_dir]/$[file] : $[file]
#define local $<
#define dest $[install_bin_dir]
	$[INSTALL_PROG]
#end file

#foreach file $[install_headers]
$[install_headers_dir]/$[file] : $[file]
#define local $<
#define dest $[install_headers_dir]
	$[INSTALL]
#end file

#foreach file $[install_parser_inc]
$[install_parser_inc_dir]/$[file] : $[file]
#define local $<
#define dest $[install_parser_inc_dir]
	$[INSTALL]
#end file

#foreach file $[install_data]
$[install_data_dir]/$[file] : $[file]
#define local $<
#define dest $[install_data_dir]
	$[INSTALL]
#end file

#foreach file $[install_config]
$[install_config_dir]/$[file] : $[file]
#define local $<
#define dest $[install_config_dir]
	$[INSTALL]
#end file


// Finally, the rules to freshen the Makefile itself.
Makefile : $[SOURCE_FILENAME]
	ppremake

#if $[and $[DEPENDENCY_CACHE_FILENAME],$[dep_sources]]
$[DEPENDENCY_CACHE_FILENAME] : $[dep_sources]
	@ppremake -D $[DEPENDENCY_CACHE_FILENAME]
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
cleanall : $[subdirs:%=cleanall-%]
install : $[if $[CONFIG_HEADER],$[install_headers_dir] $[install_headers_dir]/$[CONFIG_HEADER]] $[subdirs:%=install-%]
uninstall : $[subdirs:%=uninstall-%]
#if $[CONFIG_HEADER]
	rm -f $[install_headers_dir]/$[CONFIG_HEADER]
#endif

#formap dirname subdirs
#define depends 
$[dirname] : $[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]
	cd $[PATH]; $(MAKE) all
#end dirname

#formap dirname subdirs
test-$[dirname] :
	cd $[PATH]; $(MAKE) test
#end dirname

#formap dirname subdirs
clean-$[dirname] :
	cd $[PATH]; $(MAKE) clean
#end dirname

#formap dirname subdirs
cleanall-$[dirname] :
	cd $[PATH]; $(MAKE) cleanall
#end dirname

#formap dirname subdirs
install-$[dirname] : $[patsubst %,install-%,$[dirnames $[if $[build_directory],$[DIRNAME]],$[DEPEND_DIRS]]]
	cd $[PATH]; $(MAKE) install
#end dirname

#formap dirname subdirs
uninstall-$[dirname] :
	cd $[PATH]; $(MAKE) uninstall
#end dirname

#if $[ne $[CONFIG_HEADER],]
$[install_headers_dir] :
	@test -d $[install_headers_dir] || echo mkdir -p $[install_headers_dir]
	@test -d $[install_headers_dir] || mkdir -p $[install_headers_dir]

$[install_headers_dir]/$[CONFIG_HEADER] : $[CONFIG_HEADER]
#define local $<
#define dest $[install_headers_dir]
	$[INSTALL]
#endif

#end Makefile

// If there is a file called LocalSetup.pp in the package's top
// directory, then invoke that.  It might contain some further setup
// instructions.
#sinclude $[TOPDIRPREFIX]LocalSetup.unix.pp
#sinclude $[TOPDIRPREFIX]LocalSetup.pp

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE

//
// Global.unix.pp
//
// This file is read in before any of the individual Sources.pp files
// are read.  It defines a few global variables to assist
// Template.unix.pp.
//

#define so_dir $[ODIR_SHARED]
#define st_dir $[ODIR_STATIC]

#define install_dir $[$[upcase $[PACKAGE]]_INSTALL]
#if $[eq $[install_dir],]
  #error Variable $[upcase $[PACKAGE]]_INSTALL is not set!  Cannot install!
#endif

#define other_trees
#foreach tree $[NEEDS_TREES]
  #define tree_install $[$[upcase $[tree]]_INSTALL]
  #if $[eq $[tree_install],]
Warning: Variable $[upcase $[tree]]_INSTALL is not set!
  #else
    #set other_trees $[other_trees] $[tree_install]
  #endif
#end tree

#define install_lib_dir $[install_dir]/lib
#define install_bin_dir $[install_dir]/bin
#define install_headers_dir $[install_dir]/include
#define install_data_dir $[install_dir]/shared
#define install_igatedb_dir $[install_dir]/etc
#define install_config_dir $[install_dir]/etc

#if $[ne $[DTOOL_INSTALL],]
  #define install_parser_inc_dir $[DTOOL_INSTALL]/include/parser-inc
#else
  #define install_parser_inc_dir $[install_headers_dir]/parser-inc
#endif

#defer interrogate_options \
    -DCPPPARSER -D__cplusplus $[SYSTEM_IGATE_FLAGS] \
    -S$[install_parser_inc_dir] $[target_ipath:%=-I%] \
    $[filter -D%,$[get_cflags] $[C++FLAGS]] \
    $[INTERROGATE_OPTIONS] \
    $[if $[INTERROGATE_PYTHON_INTERFACE],-python] \
    $[if $[INTERROGATE_C_INTERFACE],-c]

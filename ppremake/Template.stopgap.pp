//
// Template.stopgap.pp
//
// This file defines the set of output files that will be generated to
// support our old-style Makefile system.  It is intended to aid as a
// transition to the new system.
//

//////////////////////////////////////////////////////////////////////
#if $[eq $[DIR_TYPE], src]
//////////////////////////////////////////////////////////////////////

// For a source directory, build a Makefile, Makefile.install, and a 
// Makefile.target for each target.

#define submakes $[TARGET(static_lib_target):%=%.a] $[TARGET(lib_target noinst_lib_target):%=%.so] $[TARGET(sed_bin_target bin_target noinst_bin_target test_bin_target)]
#define install $[TARGET(static_lib_target):%=%.a] $[TARGET(lib_target noinst_lib_target):%=%.so] $[TARGET(sed_bin_target bin_target noinst_bin_target)]


// This map variable lets us identify which metalib, if any, is
// including each library built here.
#map module COMPONENT_LIBS(*/metalib_target)

// Now iterate through the libraries we're building and see which ones
// actually *are* being included in a metalib.  For each one that is,
// we install the appropriate deferred file.
#define deferred
#forscopes lib_target
  #define metalib $[module $[TARGET],$[TARGET]]
  #if $[ne $[metalib],]
    #set deferred $[deferred] Deferred.$[metalib].lib$[TARGET].so
  #endif
#end lib_target

// Get the full set of libraries we depend on.
#call get_depend_libs

// Also get the targets we'll be installing.
#define install_libs $[sort $[TARGET(lib_target):%=lib%.so] $[TARGET(static_lib_target):%=lib%.a] $[INSTALL_LIBS]]
#define install_bin $[sort $[TARGET(bin_target)] $[INSTALL_BIN]]
#define install_scripts $[sort $[INSTALL_SCRIPTS(static_lib_target lib_target bin_target)] $[TARGET(sed_bin_target)] $[INSTALL_SCRIPTS]]
#define install_headers $[sort $[INSTALL_HEADERS(static_lib_target lib_target bin_target)] $[INSTALL_HEADERS]]
#define install_data $[sort $[INSTALL_DATA(static_lib_target lib_target sed_bin_target bin_target)] $[INSTALL_DATA]]

// Collect the set of interrogate database files we'll install,
// possibly one for each library we build.
#define install_igatedb
#if $[HAVE_PYTHON]
  #forscopes lib_target
    #if $[ne $[IGATESCAN],]
      #set install_igatedb $[install_igatedb] lib$[TARGET].in
    #endif
  #end lib_target
#endif


#output Makefile
#format makefile
#### Meta Makefile.
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################


#### Sub make targets (extension of sub Makefile, eg 'foo' for Makefile.foo):
SUBMAKES = $[submakes]

#### List the minimal set of sub makes on the list above required to install.
INSTALL = $[install]

#### Location of sub Makefiles.
MAKEDIR = .

#### The action is here.
include $(DTOOL)/inc/Makefile.meta.rules

#### Sub-make build order dependencies:
# foo: bar
#end Makefile



#output Makefile.install
#format makefile
#### Installation makefile
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

# Note: This file is included by the project-wide Makefile so the current
# directory is the project root.  Also, commented-out fields are optional.

#### Package name and location (if not src/all/$(PACKAGE)):
PACKAGE = $[DIRNAME]
PKGROOT = $[PATH]

ifneq (,$(PACKAGE))

#### Package dependencies (USESOTHER needs relative paths from project root):
USESLIBS = $[depend_libs]
# USESINCLUDE = 
# USESOTHER = 

#### Installed files:
LIBS = $[install_libs]
DEFERRED = $[deferred]
INCLUDE = $[install_headers]
BINS = $[install_bin]
SCRIPTS = $[install_scripts]
# SS = 
# STK = 
# MODELS = 
ETC = $[install_data]
IGATEDB =$[install_igatedb]
# DOC =
# MAN =
# FONTS = 
# ICONS = 
# APPDEFAULTS = 
# TCL = 
# TELEUSE = 
# SHADERS = 

#### Other files to be installed (use relative pathname from project root):
#if $[ne $[INSTALL_PARSER_INC],]
PARSER_INC = $[INSTALL_PARSER_INC]
SRC_PARSER_INC = $(addprefix $(PKGROOT)/,$(PARSER_INC))
INST_PARSER_INC = $(addprefix inc/parser-inc/,$(PARSER_INC))
OTHER = $(INST_PARSER_INC)
#else
# OTHER =
#endif

#### Where the action happens.
include $(DTOOL)/inc/Makefile.install.rules

#### Install actions for OTHER files (source must be in $(PKGROOT)):
# [ installed file ] : $(PKGROOT)/[ source file ]  # Files must have same name
#	$(INSTALL)                      # Copies from source to dest
#
# [ installed file ] : $(PKGROOT)/[ source file ]
#	$(MKINSTALL)			# Also makes directory if needed

#if $[ne $[INSTALL_PARSER_INC],]
$(INST_PARSER_INC) : inc/parser-inc/% : $(PKGROOT)/%
	$(MKINSTALL)
#endif

#### Other install/uninstall actions:
# install-$(PKGROOT): #Add dependencies here
#	Add actions here
#
# uninstall-$(PKGROOT): #Add dependencies here
#	Add actions here

#### Sub-package Makefile.install inclusions:
# include foo/Makefile.install

endif
#end Makefile.install



// Now generate a suitable Makefile for each library target.
#forscopes lib_target noinst_lib_target

// Again, is this library included in a metalib?  If so, output the
// appropriate deferred rules.
#define metalib $[module $[TARGET],$[TARGET]]

// We might need to define a BUILDING_ symbol for win32.  We use the
// BUILDING_DLL variable name, defined typically in the metalib, for
// this; but in some cases, where the library isn't part of a metalib,
// we define BUILDING_DLL directly for the target.
#define building_var $[BUILDING_DLL]
#if $[ne $[metalib],]
  #set building_var $[module $[BUILDING_DLL],$[TARGET]]
#endif

// Get the full set of sources for this target.
#call get_sources
#call get_libs

// Which files will we interrogate, if any?
#if $[HAVE_PYTHON]
  #if $[ne $[IGATESCAN],]
    #if $[eq $[IGATESCAN],all]
      #define igatescan $[filter-out %.I %.lxx %.yxx %.N,$[sources]]
    #else
      #define igatescan $[IGATESCAN]
    #endif
  #endif
#endif

#output Makefile.$[TARGET].so
#format makefile
#### Makefile for DSO's.  Any fields commented out are optional.
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#### Target's name:
TARGET = lib$[TARGET].so
DEFERRED_TARGET = $[metalib]

# Standard .o file conversion information.

#### Lex files 
LFILES = $[filter %.lxx,$[sources]]
LEX = flex
LFLAGS = $[LFLAGS] $[YACC_PREFIX:%=-P%] -olex.yy.c
LEXTENSION = cxx

#### Yacc files 
YFILES = $[filter %.yxx,$[sources]]
YACC = bison
YFLAGS = -y -d $[patsubst %,--name-prefix=%,$[YACC_PREFIX]]
YEXTENSION = cxx

#### C files 
CFILES = $[filter %.c,$[sources]]
CFLAGS = $[building_var:%=-D%] $[alt_cflags] $[CFLAGS]

#### C++ files 
C++FILES = $[filter %.cxx,$[sources]]
C++FLAGS = $[building_var:%=-D%] $[alt_cflags] $[C++FLAGS]
# USETEMPLATES = TRUE
# PTREPOSITORY = # Specify only if you want a specific name

#### Interrogate info
IGATESCAN  = $[igatescan]
IGATEFLAGS = $[alt_ipath]
# IGATEFILE  = # Specify only if you want a specific name

#### Additional search directories for C/C++ header files:
IPATH = $[alt_ipath]

#### Location to put .o files:
# ODIR = 

#### Source file dependencies (unnecessary with clearmake)
# foo.c: foo.h

#### Other files and lib.  Include $(ODIR) in any .o names.
# OFILES = 
WHEN_NO_DEFER_LIBS = $[when_no_defer:%=-l%]
WHEN_DEFER_LIBS = $[when_defer:%=-l%]
LIBS = $[when_either:%=-l%]
SYSLIBS = $[patsubst %.lib,%.lib,%,-l%,$[unique $[alt_libs]]]

#### Additional search directories for lib:
LPATH = $[alt_lpath]

#### Other linker flags. 
#if $[ne $[alt_ld],]
LD = $[alt_ld]
#endif
# LDFLAGS = 

#### Pull in standard .o make variables
include $(DTOOL)/inc/Makefile.o.vars

#### The .o action is here.
include $(DTOOL)/inc/Makefile.o.rules

#### Pull in standard binary make variables.
include $(DTOOL)/inc/Makefile.bin.vars

#### The .so action is here.
include $(DTOOL)/inc/Makefile.so.rules
#end Makefile.$[TARGET].so

#end lib_target noinst_lib_target



// Also generate a suitable Makefile for each static library target.
#forscopes static_lib_target

// Get the full set of sources for this target.
#call get_sources
#call get_libs

#output Makefile.$[TARGET].a
#format makefile
#### Makefile for archive libraries.  Any fields commented out are optional.
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#### Target's name:
TARGET = lib$[TARGET].a

# Standard .o file conversion information.

#### Lex files 
LFILES = $[filter %.lxx,$[sources]]
LEX = flex
LFLAGS = $[LFLAGS] $[YACC_PREFIX:%=-P%] -olex.yy.c
LEXTENSION = yy.cxx
# LSUBST =

#### Yacc files 
YFILES = $[filter %.yxx,$[sources]]
YACC = bison
YFLAGS = -y -d $[patsubst %,--name-prefix=%,$[YACC_PREFIX]]
YEXTENSION = tab.cxx
# YSUBST =

#### C files 
CFILES = $[filter %.c,$[sources]]
CFLAGS = $[building_var:%=-D%] $[alt_cflags] $[CFLAGS]

#### C++ files 
C++FILES = $[filter %.cxx,$[sources]]
C++FLAGS = $[building_var:%=-D%] $[alt_cflags] $[C++FLAGS]
# USETEMPLATES = TRUE
# PTREPOSITORY = # Specify only if you want a specific name

#### Additional search directories for C/C++ header files:
IPATH = $[alt_ipath]

#### Location to put .o files:
# ODIR = 

#### Source file dependencies (unnecessary with clearmake)
# foo.c: foo.h

#### Other .o files.
# OFILES = 

#### Libs and flags for template instantiation.
WHEN_NO_DEFER_LIBS = $[when_no_defer:%=-l%]
WHEN_DEFER_LIBS = $[when_defer:%=-l%]
LIBS = $[when_either:%=-l%]
SYSLIBS = $[patsubst %.lib,%.lib,%,-l%,$[unique $[alt_libs]]]

#### Additional search directories for lib:
LPATH = $[alt_lpath]

#### Archiver flags
# ARFLAGS = 

#### Pull in standard .o make variables
include $(DTOOL)/inc/Makefile.o.vars

#### The .o action is here.
include $(DTOOL)/inc/Makefile.o.rules

#### Pull in standard binary make variables.
include $(DTOOL)/inc/Makefile.bin.vars

#### The .a action is here.
include $(DTOOL)/inc/Makefile.a.rules
#end Makefile.$[TARGET].a

#end static_lib_target



// And also generate a suitable Makefile for each binary target.
#forscopes bin_target noinst_bin_target test_bin_target

// Get the full set of sources for this target.
#call get_sources
#call get_libs

#output Makefile.$[TARGET]
#format makefile
#### Makefile for binaries.  Any fields commented out are optional.
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#### Target's name:
TARGET = $[TARGET]

# Standard .o file conversion information.

#### Lex files 
LFILES = $[filter %.lxx,$[sources]]
LEX = flex
LFLAGS = $[LFLAGS] $[YACC_PREFIX:%=-P%] -olex.yy.c
LEXTENSION = yy.cxx
# LSUBST =

#### Yacc files 
YFILES = $[filter %.yxx,$[sources]]
YACC = bison
YFLAGS = -y -d $[patsubst %,--name-prefix=%,$[YACC_PREFIX]]
YEXTENSION = tab.cxx
# YSUBST =

#### C files 
CFILES = $[filter %.c,$[sources]]
CFLAGS = $[building_var:%=-D%] $[alt_cflags] $[CFLAGS]

#### C++ files 
C++FILES = $[filter %.cxx,$[sources]]
C++FLAGS = $[building_var:%=-D%] $[alt_cflags] $[C++FLAGS]

#### Additional search directories for C/C++ header files:
IPATH = $[alt_ipath]

#### Location to put .o files:
# ODIR = 

#### Source file dependencies (unnecessary with clearmake)
# foo.c: foo.h

#### Other files and lib.  Include $(ODIR) in any .o names.
# OFILES = 
WHEN_NO_DEFER_LIBS = $[when_no_defer:%=-l%]
WHEN_DEFER_LIBS = $[when_defer:%=-l%]
LIBS = $[when_either:%=-l%]
SYSLIBS = $[patsubst %.lib,%.lib,%,-l%,$[unique $[alt_libs]]]

#### Additional search directories for lib:
LPATH = $[alt_lpath]

#### Other linker flags. 
#if $[ne $[alt_ld],]
LD = $[alt_ld]
#endif
# LDFLAGS =

#### Pull in standard .o make variables
include $(DTOOL)/inc/Makefile.o.vars

#### The .o action is here.
include $(DTOOL)/inc/Makefile.o.rules

#### Pull in standard binary make variables.
include $(DTOOL)/inc/Makefile.bin.vars

#### The bin action is here.
include $(DTOOL)/inc/Makefile.bin.rules
#end Makefile.$[TARGET]

#end bin_target noinst_bin_target test_bin_target


// Finally, generate the special scripts from the sed_bin_targets.  Hopefully
// there won't be too many of these in the tree, since these are fairly
// Unix-specific.
#forscopes sed_bin_target
#output Makefile.$[TARGET]
#format makefile
#### This is a special makefile just to generate the $[TARGET] script.

$[TARGET] : $[SOURCE]
	sed $[COMMAND] $^ >$@
	chmod +x $@

clean :

cleanall :
	rm -f $[TARGET]
#end Makefile.$[TARGET]
#end sed_bin_target


//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], metalib]
//////////////////////////////////////////////////////////////////////

// A metalib directory is similar to a regular source directory, 
// but a little simpler.

#define submakes $[TARGET(metalib_target):%=%.so]


// This map variable lets us identify which metalib, if any, is
// including each library built here.
#map module COMPONENT_LIBS(*/metalib_target)

// Get the full set of libraries we depend on.
#call get_depend_libs

// Also get the targets we'll be installing.
#define install_libs $[TARGET(metalib_target):%=lib%.so]
#define install_headers $[INSTALL_HEADERS(metalib_target)]
#define install_data $[INSTALL_DATA(metalib_target)]


#output Makefile
#format makefile
#### Meta Makefile.
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################


#### Sub make targets (extension of sub Makefile, eg 'foo' for Makefile.foo):
SUBMAKES = $[submakes]

#### List the minimal set of sub makes on the list above required to install.
INSTALL = $[submakes]

#### Location of sub Makefiles.
MAKEDIR = .

#### The action is here.
include $(DTOOL)/inc/Makefile.meta.rules

#### Sub-make build order dependencies:
# foo: bar
#end Makefile



#output Makefile.install
#format makefile
#### Installation makefile
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

# Note: This file is included by the project-wide Makefile so the current
# directory is the project root.  Also, commented-out fields are optional.

#### Package name and location (if not src/all/$(PACKAGE)):
PACKAGE = $[DIRNAME]
PKGROOT = $[PATH]

ifneq (,$(PACKAGE))

#### Package dependencies (USESOTHER needs relative paths from project root):
USESLIBS = $[depend_libs]
# USESINCLUDE = 
# USESOTHER = 

#### Installed files:
LIBS = $[install_libs]
INCLUDE = $[install_headers]
# BINS =
# SS = 
# STK = 
# MODELS = 
# ETC =
# DOC =
# MAN =
# FONTS = 
# ICONS = 
# APPDEFAULTS = 
# TCL = 
# TELEUSE = 
# SHADERS = 

#### Other files to be installed (use relative pathname from project root):
# OTHER = 

#### Where the action happens.
include $(DTOOL)/inc/Makefile.install.rules

#### Install actions for OTHER files (source must be in $(PKGROOT)):
# [ installed file ] : $(PKGROOT)/[ source file ]  # Files must have same name
#	$(INSTALL)                      # Copies from source to dest
#
# [ installed file ] : $(PKGROOT)/[ source file ]
#	$(MKINSTALL)			# Also makes directory if needed

#### Other install/uninstall actions:
# install-$(PKGROOT): #Add dependencies here
#	Add actions here
#
# uninstall-$(PKGROOT): #Add dependencies here
#	Add actions here

#### Sub-package Makefile.install inclusions:
# include foo/Makefile.install

endif
#end Makefile.install

// Now generate a suitable Makefile for each metalib target.
#forscopes metalib_target

#define building_var $[BUILDING_DLL]

// Get the full set of sources for this target.
#call get_sources
#call get_libs

#if $[HAVE_PYTHON]
  #map components TARGET(*/lib_target */noinst_lib_target)
  #if $[ne $[components $[IGATESCAN],$[COMPONENT_LIBS]],]
    #define igatemscan $[TARGET]
  #endif
#endif

#output Makefile.$[TARGET].so
#format makefile
#### Makefile for DSO's.  Any fields commented out are optional.
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

#### Target's name:
TARGET = lib$[TARGET].so

# Standard .o file conversion information.

#### Lex files 
LFILES = $[filter %.lxx,$[sources]]
LEX = flex
LFLAGS = $[LFLAGS] $[YACC_PREFIX:%=-P%] -olex.yy.c
LEXTENSION = yy.cxx
# LSUBST =

#### Yacc files 
YFILES = $[filter %.yxx,$[sources]]
YACC = bison
YFLAGS = -y -d $[patsubst %,--name-prefix=%,$[YACC_PREFIX]]
YEXTENSION = tab.cxx
# YSUBST =

#### C files 
CFILES = $[filter %.c,$[sources]]
CFLAGS = $[building_var:%=-D%] $[alt_cflags] $[CFLAGS]

#### C++ files 
C++FILES = $[filter %.cxx,$[sources]]
C++FLAGS = $[building_var:%=-D%] $[alt_cflags] $[C++FLAGS]
# USETEMPLATES = TRUE
# PTREPOSITORY = # Specify only if you want a specific name

#### Interrogate info
# IGATESCAN  = 
# IGATEFLAGS = 
# IGATEFILE  = # Specify only if you want a specific name
IGATEMSCAN = $[igatemscan]

#### Pull in deferred-target files built in other packages
DEFERRED_FILES = $[TARGET]

#### Additional search directories for C/C++ header files:
IPATH = $[alt_ipath]

#### Location to put .o files:
# ODIR = 

#### Source file dependencies (unnecessary with clearmake)
# foo.c: foo.h

#### Other files and lib.  Include $(ODIR) in any .o names.
# OFILES = 
WHEN_NO_DEFER_LIBS = $[when_no_defer:%=-l%]
WHEN_DEFER_LIBS = $[when_defer:%=-l%]
LIBS = $[when_either:%=-l%]
SYSLIBS = $[patsubst %.lib,%.lib,%,-l%,$[unique $[alt_libs]]]

#### Additional search directories for lib:
LPATH = $[alt_lpath]

#### Other linker flags. 
#if $[ne $[alt_ld],]
LD = $[alt_ld]
#endif
# LDFLAGS = 

#### Pull in standard .o make variables
include $(DTOOL)/inc/Makefile.o.vars

#### The .o action is here.
include $(DTOOL)/inc/Makefile.o.rules

#### Pull in standard binary make variables.
include $(DTOOL)/inc/Makefile.bin.vars

#### The .so action is here.
include $(DTOOL)/inc/Makefile.so.rules
#end Makefile.$[TARGET].so

#end metalib_target


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

#output Makefile
#format makefile
#### Generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
################################# DO NOT EDIT ###########################

# Specify project name and project root directory.
CTPROJECT = $[PACKAGE]
CTPROJROOT = $($[upcase $[PACKAGE]])

include $(DTOOL)/inc/Makefile.project.vars

// Iterate through all of our known source files.  Each src and
// metalib type file gets its corresponding Makefile.install listed
// here.  However, we test for $[DIR_TYPE] of toplevel, because the
// source directories typically don't define their own DIR_TYPE
// variable, and they end up inheriting this one dynamically.
#forscopes */
#if $[or $[eq $[DIR_TYPE], src],$[eq $[DIR_TYPE], metalib],$[and $[eq $[DIR_TYPE], toplevel],$[ne $[DIRNAME],top]]]
#if $[build_directory]
include $[PATH]/Makefile.install
#endif
#endif
#end */

#end Makefile

// If there is a file called LocalSetup.pp in the package's top
// directory, then invoke that.  It might contain some further setup
// instructions.
#sinclude $[TOPDIRPREFIX]LocalSetup.stopgap.pp
#sinclude $[TOPDIRPREFIX]LocalSetup.pp

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE

//
// Template.autoconf.pp
//
// This file defines the set of output files that will be generated to
// support an autoconf/automake style build.  This works particularly
// well when gcc/g++ will be used to compile, for instance on Linux.
//

#defer get_sys_libs $[subst -ldl,@libdl@,$[patsubst %,-l%,$[UNIX_SYS_LIBS]]]

// First, check to see if the entire directory has been switched out.
#define omit
#if $[DIRECTORY_IF_GL]
  #set omit $[not $[HAVE_GL]]
#endif
#if $[DIRECTORY_IF_GLX]
  #set omit $[not $[HAVE_GLX]]
#endif
#if $[DIRECTORY_IF_WGL]
  #set omit $[not $[HAVE_WGL]]
#endif
#if $[DIRECTORY_IF_GLUT]
  #set omit $[not $[HAVE_GLUT]]
#endif
#if $[DIRECTORY_IF_SGIGL]
  #set omit $[not $[HAVE_SGIGL]]
#endif
#if $[DIRECTORY_IF_DX]
  #set omit $[not $[HAVE_DX]]
#endif
#if $[DIRECTORY_IF_PS2]
  #set omit $[not $[HAVE_PS2]]
#endif
#if $[DIRECTORY_IF_RIB]
  #set omit $[not $[HAVE_RIB]]
#endif
#if $[DIRECTORY_IF_VRPN]
  #set omit $[not $[HAVE_VRPN]]
#endif

//////////////////////////////////////////////////////////////////////
#if $[eq $[DIR_TYPE], src]
//////////////////////////////////////////////////////////////////////

// For a source directory, build a Makefile.am with a number of targets.

#output Makefile.am
#format makefile
# Makefile.am generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].

#if $[omit]
  // If we're omitting the directory, everything becomes an extra_dist.  
EXTRA_DIST = Sources.pp $[EXTRA_DIST] $[SOURCES(static_lib_target noinst_lib_target lib_target noinst_bin_target bin_target test_bin_target)]

#else   // $[omit]

// We define a map variable that allows us to look up all the libs in
// the various directories by target name.  With this map variable, we
// can translate the list of local_libs (which is simply a list of
// library names, with no directory information), into a list of
// relative filenames to each library.
#map local_libs TARGET(*/static_lib_target */lib_target */noinst_lib_target)

#define all_include_dirs

lib_LTLIBRARIES = $[TARGET(static_lib_target lib_target):%=lib%.la]
noinst_LTLIBRARIES = $[TARGET(noinst_lib_target):%=lib%.la]
bin_PROGRAMS = $[TARGET(bin_target)]
noinst_PROGRAMS = $[TARGET(noinst_bin_target)]
EXTRA_PROGRAMS = $[TARGET(test_bin_target)]

#if $[ne $[YACC_PREFIX],]
YFLAGS = -d --name-prefix=$[YACC_PREFIX] $[YFLAGS]
LFLAGS = -P$[YACC_PREFIX] -olex.yy.c $[LFLAGS]
#else
YFLAGS = -d $[YFLAGS]
LFLAGS = $[LFLAGS]
#endif

#define alt_cflags @nspr_cflags@ @python_cflags@
#define alt_lflags @nspr_lflags@
#define alt_libs @nspr_libs@
#if $[ne $[USE_ZLIB],]
  #define alt_cflags $[alt_cflags] @zlib_cflags@
  #define alt_lflags $[alt_lflags] @zlib_lflags@
  #define alt_libs $[alt_libs] @zlib_libs@
#endif
#if $[ne $[USE_GL],]
  #define alt_cflags $[alt_cflags] @gl_cflags@ @glut_cflags@
  #define alt_lflags $[alt_lflags] @gl_lflags@ @glut_lflags@
  #define alt_libs $[alt_libs] @gl_libs@ @glut_libs@
#endif

#define built_sources
#define install_data

#forscopes static_lib_target lib_target noinst_lib_target

// This map variable lets us identify which metalib, if any, is
// including this particular library.
#map module LOCAL_LIBS(*/metalib_target)

// And this defines the complete set of libs we depend on: the
// LOCAL_LIBS we listed as directly depending on, plus all of the
// LOCAL_LIBS *those* libraries listed, and so on.
#define complete_local_libs $[closure local_libs,$[LOCAL_LIBS]]


#if $[ne $[IF_ZLIB_SOURCES],]
if HAVE_ZLIB
EXTRA_ZLIB = $[IF_ZLIB_SOURCES]
else
EXTRA_ZLIB =
endif
#define SOURCES $[SOURCES] $(EXTRA_ZLIB)
#endif

#define local_incs $[local_libs $[RELDIR],$[complete_local_libs]] $[RELDIR($[LOCAL_INCS:%=%/])]

// Check for interrogate.
#if $[eq $[IGATESCAN], all]
  #define IGATESCAN $[filter-out %.I %.lxx %.yxx %.N,$[SOURCES]]
#endif
#if $[ne $[IGATESCAN],]
  #define IGATEFILE $[TARGET].in.cxx
  #define IGATEDBFILE lib$[TARGET].in

  #define IGATELIBRARY lib$[TARGET]
  #define IGATEMODULE lib$[module $[TARGET],$[TARGET]]
  #if $[eq $[IGATEMODULE], lib]
    #define IGATEMODULE $[IGATELIBRARY]
  #endif

IGATESCAN = $[IGATESCAN]
$[IGATEFILE] : $(IGATESCAN)
	@dtool@/bin/interrogate $[IGATEFLAGS] @system_igate@ -DCPPPARSER -D__cplusplus -I@dtool@/include/parser-inc @trees_inc@ $[local_incs:%=-I%] $[alt_cflags] $[CDEFINES] -module "$[IGATEMODULE]" -library "$[IGATELIBRARY]" -oc $[IGATEFILE] -od $[IGATEDBFILE] -fnames -string -refcount -assert -promiscuous -python $(IGATESCAN)
#set built_sources $[built_sources] $[IGATEFILE]
#set install_data $[install_data] $[IGATEDBFILE]
#endif

  #define SOURCES $[SOURCES] $[IGATEFILE]

lib$[TARGET]_la_SOURCES = $[SOURCES]
lib$[TARGET]_la_LIBADD = $[OTHER_LIBS:%=-l%] $[alt_libs] $[get_sys_libs]

#set all_include_dirs $[all_include_dirs] $[local_incs]
  


#end static_lib_target lib_target noinst_lib_target

#forscopes bin_target noinst_bin_target test_bin_target
#if $[ne $[IF_ZLIB_SOURCES],]
if HAVE_ZLIB
EXTRA_ZLIB = $[IF_ZLIB_SOURCES]
else
EXTRA_ZLIB =
endif
#define $[SOURCES] $(EXTRA_ZLIB)
#endif

// This defines the complete set of libs we depend on: the LOCAL_LIBS
// we listed as directly depending on, plus all of the LOCAL_LIBS
// *those* libraries listed, and so on.
#define complete_local_libs $[closure local_libs,$[LOCAL_LIBS]]

$[TARGET]_SOURCES = $[SOURCES]
$[TARGET]_LDADD = $[local_libs $[RELDIR]/lib$[TARGET].la,$[complete_local_libs]] $[OTHER_LIBS:%=-l%] $[alt_libs] $[get_sys_libs]
#set all_include_dirs $[all_include_dirs] $[local_libs $[RELDIR],$[complete_local_libs]]
#set all_include_dirs $[all_include_dirs] $[RELDIR($[LOCAL_INCS:%=%/])]

#end bin_target noinst_bin_target test_bin_target

include_HEADERS = $[sort $[INSTALL_HEADERS(static_lib_target lib_target bin_target)] $[INSTALL_HEADERS]]
#set install_data $[install_data] $[INSTALL_DATA]

#if $[ne $[INSTALL_PARSER_INC],]
parserincdir = @includedir@/parser-inc
parserinc_HEADERS = $[INSTALL_PARSER_INC]
#endif

INCLUDES = $[patsubst %,-I%,$[sort $[all_include_dirs]]] @trees_inc@ $[alt_cflags]
LDFLAGS = @ldflags@ @trees_lflags@ $[alt_lflags]
EXTRA_DIST = Sources.pp $[EXTRA_DIST] $[INSTALL_LIBS] $[INSTALL_SCRIPTS]$[install_data]
BUILT_SOURCES =$[built_sources]
data_DATA =$[install_data]

#endif   // $[omit]

#end Makefile.am


//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], metalib]
//////////////////////////////////////////////////////////////////////

// A metalib directory is similar to a regular source directory, 
// but a little simpler.

#output Makefile.am
#format makefile
# Makefile.am generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].

#if $[omit]
  // If we're omitting the directory, everything becomes an extra_dist.  
EXTRA_DIST = Sources.pp $[EXTRA_DIST] $[SOURCES(static_lib_target noinst_lib_target lib_target noinst_bin_target bin_target test_bin_target)]

#else   // $[omit]

// We define a map variable that allows us to look up all the libs in
// the various directories by target name.  With this map variable, we
// can translate the list of local_libs (which is simply a list of
// library names, with no directory information), into a list of
// relative filenames to each library.
#map local_libs TARGET(*/static_lib_target */lib_target */noinst_lib_target)

#define all_include_dirs $(includedir)

lib_LTLIBRARIES = $[TARGET(metalib_target):%=lib%.la]

#define alt_cflags @nspr_cflags@ @python_cflags@
#define alt_lflags @nspr_lflags@
#define alt_libs @nspr_libs@
#if $[ne $[USE_ZLIB],]
  #define alt_cflags $[alt_cflags] @zlib_cflags@
  #define alt_lflags $[alt_lflags] @zlib_lflags@
  #define alt_libs $[alt_libs] @zlib_libs@
#endif
#if $[ne $[USE_GL],]
  #define alt_cflags $[alt_cflags] @gl_cflags@
  #define alt_lflags $[alt_lflags] @gl_lflags@
  #define alt_libs $[alt_libs] @gl_libs@
#endif

#forscopes metalib_target
#if $[ne $[IF_PYTHON_SOURCES],]
if HAVE_PYTHON
EXTRA_PYTHON = $[IF_PYTHON_SOURCES]
else
EXTRA_PYTHON =
endif
#define SOURCES $[SOURCES] $(EXTRA_PYTHON)
#endif

// This defines the complete set of libs we depend on: the LOCAL_LIBS
// we listed as directly depending on, plus all of the LOCAL_LIBS
// *those* libraries listed, and so on.
#define complete_local_libs $[closure local_libs,$[LOCAL_LIBS]]

lib$[TARGET]_la_SOURCES = $[SOURCES]
lib$[TARGET]_la_LIBADD = $[complete_local_libs:%=-l%] $[get_sys_libs]

#end metalib_target

INCLUDES = $[patsubst %,-I%,$[sort $[all_include_dirs]]] @trees_inc@ $[alt_cflags]
LDFLAGS = @ldflags@ -L$(libdir) -rpath $(libdir) @trees_lflags@ $[alt_lflags]
EXTRA_DIST = Sources.pp $[EXTRA_DIST]

#endif   // $[omit]

#end Makefile.am


//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], group]
//////////////////////////////////////////////////////////////////////

#output Makefile.am
#format makefile
# Makefile.am generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].

SUBDIRS = $[SUBDIRS]

EXTRA_DIST = Sources.pp $[EXTRA_DIST]

#end Makefile.am



//////////////////////////////////////////////////////////////////////
#elif $[eq $[DIR_TYPE], toplevel]
//////////////////////////////////////////////////////////////////////

#output Makefile.am
#format makefile
# Makefile.am generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].

FIRSTBUILD_SUBDIRS = $[filter_out metalibs,$[SUBDIRS]]
SUBDIRS = $[SUBDIRS]

#define INSTALL_HEADERS $[INSTALL_HEADERS] $[CONFIG_HEADER]

include_HEADERS = $[INSTALL_HEADERS]

EXTRA_DIST = Sources.pp Config.pp $[EXTRA_DIST]


# We define this custom rule for all-recursive as an ordering hack.
# It's just like the default rule, except that it traverses through
# only FIRSTBUILD_SUBDIRS, instead of all of SUBDIRS.  The idea is
# that first we build everything in FIRSTBUILD_SUBDIRS, and then when
# we're installing, we build everything in SUBDIRS as well.  This hack
# is necessary to build targets in metalibs that link directly with
# installed shared libraries.

all-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(FIRSTBUILD_SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

#end Makefile.am

#output configure.in
#format straight
dnl configure.in generated automatically by $[PROGRAM] $[PROGVER] from $[SOURCEFILE].
dnl Process this file with autoconf to produce a configure script.
AC_INIT($[SAMPLE_SOURCE_FILE])
AM_INIT_AUTOMAKE($[PACKAGE], $[VERSION])

#if $[eq $[CONFIG_HEADER],]
dnl This package doesn't care about a generated config.h file.  This causes
dnl a few problems with automake, so we fake it out with this hack.
AM_CONFIG_HEADER(ignore_config.h)
#else
AM_CONFIG_HEADER($[CONFIG_HEADER])
#endif

if test "${CTPROJS+set}" = set; then
  if test "${$[upcase $[PACKAGE]]+set}" != set; then
    echo ""
    echo "The environment variable CTPROJS is currently set, indicating"
    echo "you are attached to one or more trees, but you are not attached"
    echo "to $[upcase $[PACKAGE]].  Either unattach from everything, or attach to $[upcase $[PACKAGE]]."
    echo ""
    exit 1
  fi

  # If we're currently attached, the install directory is the same as
  # the source directory.
  if test "$prefix" != "NONE"; then
    echo ""
    echo The environment variable CTPROJS is currently set, indicating
    echo you are attached to one or more trees.  When you configure the
    echo package while attached, you cannot specify a --prefix to install
    echo the built sources--it always installs in the source directory.
    echo ""
    exit 1
  fi

  prefix=$$[upcase $[PACKAGE]]
fi

AC_PREFIX_DEFAULT($[INSTALL_DIR])
AC_PROG_MAKE_SET
AC_CANONICAL_HOST

# If we have a CFLAGS variable but not a CXXFLAGS variable, let them
# be the same.
if test "${CXXFLAGS+set}" != set -a "${CFLAGS+set}" = set; then
  CXXFLAGS=$CFLAGS
fi

# Save these variables for later, so we can easily append to them or
# change them.
user_ldflags=${LDFLAGS-}
user_cflags=${CFLAGS-}
user_cxxflags=${CXXFLAGS-}

dnl Choose a suitable set of system-dependent interrogate flags.
case "$host_os" in
  irix*) system_igate="-D__mips__ -D__MIPSEB__";;
  linux-gnu*) system_igate="-D__i386__";;
esac
AC_SUBST(system_igate)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_LEX
AM_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_PATH_XTRA

dnl We require specifically Bison, not any other flavor of Yacc.
AC_CHECK_PROGS(YACC, 'bison -y')


AC_ARG_WITH(optimize,
[  --with-optimize=LEVEL   Specify the optimization/debug symbol level (1-4, default 1)])

if test "${with_optimize-no}" = "no"; then
  with_optimize=$[OPTIMIZE]
fi

if test "$with_optimize" = "1"; then
  # Optimize level 1: No optimizations, and full debug symbols.  
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="$user_cflags -g -Wall"
    CXXFLAGS="$user_cxxflags -g -Wall"
  else
    CFLAGS="$user_cflags -g"
    CXXFLAGS="$user_cxxflags -g"
  fi

elif test "$with_optimize" = "2"; then
  # Optimize level 2: Compiler optimizations, and full debug if supported.
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="$user_cflags -O2 -g -Wall"
    CXXFLAGS="$user_cxxflags -O2 -g -Wall"
  else
    CFLAGS="$user_cflags -O"
    CXXFLAGS="$user_cxxflags -O"
  fi

elif test "$with_optimize" = "3"; then
  # Optimize level 3: Compiler optimizations, without debug symbols.
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="$user_cflags -O2 -Wall"
    CXXFLAGS="$user_cxxflags -O2 -Wall"
  else
    CFLAGS="$user_cflags -O"
    CXXFLAGS="$user_cxxflags -O"
  fi

elif test "$with_optimize" = "4"; then
  # Optimize level 4: As above, with asserts removed.
  if test "${ac_cv_prog_gcc}" = "yes"; then
    CFLAGS="$user_cflags -O2 -Wall -DNDEBUG"
    CXXFLAGS="$user_cxxflags -O2 -Wall -DNDEBUG"
  else
    CFLAGS="$user_cflags -O -DNDEBUG"
    CXXFLAGS="$user_cxxflags -O -DNDEBUG"
  fi

else
  echo "Invalid optimize level: $with_optimize"
  exit 0
fi

trees_inc=
trees_lflags=

#foreach require $[REQUIRED_TREES]

AC_ARG_WITH($[require],
[  --with-$[require]=DIR        Prefix where $[upcase $[require]] is installed (same as --prefix)])


if test "${CTPROJS+set}" = set; then
  if test "$with_$[require]" != ""; then
    echo ""
    echo "The environment variable CTPROJS is currently set, indicating"
    echo "you are attached to one or more trees.  When you configure the"
    echo "package while attached, you cannot specify a directory to search"
    echo "for --$[require]; it will always search in the currently attached $[upcase $[require]]."
    echo ""
    exit 1
  fi
  if test "${$[upcase $[require]]+set}" != set; then
    echo ""
    echo "The environment variable CTPROJS is currently set, indicating"
    echo "you are attached to one or more trees, but you are not attached"
    echo "to $[upcase $[require]].  Either unattach from everything, or attach to $[upcase $[require]]."
    echo ""
    exit 1
  fi

  $[require]='${$[upcase $[require]]}'
else
  # No attachments--respect the --with-$[require] parameter.

  if test "$with_$[require]" != "" -a "$with_$[require]" != "no" -a "$with_$[require]" != "yes"; then
    $[require]=$with_$[require]
  else
    $[require]='${prefix}'
  fi
  trees_inc="$trees_inc -I"'$($[require])/include'
  trees_lflags="$trees_lflags -L"'$($[require])/lib'
fi

AC_SUBST($[require])

#end require
AC_SUBST(trees_inc)
AC_SUBST(trees_lflags)

dnl First, we'll test for C-specific features.
AC_LANG_C

dnl Checks for libraries.
libdl=
libm=
AC_CHECK_LIB(dl, dlopen, libdl=-ldl)
AC_CHECK_LIB(m, sin, libm=-lm)
AC_SUBST(libdl)
AC_SUBST(libm)

// Only bother to make the following tests if we're actually building
// a config.h.
#if $[ne $[CONFIG_HEADER],]
dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(malloc.h alloca.h unistd.h io.h minmax.h sys/types.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_GETTIMEOFDAY

dnl Checks for library functions.
AC_CHECK_FUNCS(getopt getopt_long_only)


dnl Now we can test some C++-specific features.
AC_LANG_CPLUSPLUS
AC_HEADER_IOSTREAM
AC_CHECK_HEADERS(sstream)
AC_NAMESPACE
AC_IOS_BINARY
#endif

AC_LANG_C

AC_ARG_WITH(python,
[  --with-python=DIR       Prefix where Python is installed (usually /usr/local)])

have_python=no
include_python=
if test "$with_python" != "no"; then
  if test "$with_python" = "yes" -o "$with_python" = ""; then
    AC_SEARCH_HPACKAGE(/usr/local /usr,
                       python1.6 python,
	               Python.h, python)
  else
    AC_SEARCH_HPACKAGE($with_python,
                       python1.6 python,
	               Python.h, python)
  fi

  if test "$with_python" != ""; then
    if test "$python_PKG" != "yes"; then
      dnl  If the user specified to search for python but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate Python package.  Use --with-python=directory,"
      echo "     e.g. --with-python=/usr/local, or just --with-python."
      echo "     If Python is not installed, specify --without-python."
      echo ""
      exit 1
    fi
  fi

  if test "$python_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_PYTHON)
#endif
    have_python=yes
    python_cflags=-I$python_INCLUDE
  fi
fi

AC_SUBST(have_python)
AC_SUBST(python_cflags)
AM_CONDITIONAL(HAVE_PYTHON, test "$have_python" = "yes")


AC_ARG_WITH(nspr,
[  --with-nspr=DIR         Prefix where NSPR is installed (usually /usr/local/mozilla)])

have_nspr=no
include_nspr=
if test "$with_nspr" != "no"; then
  if test "$with_nspr" = "yes" -o "$with_nspr" = ""; then
    AC_SEARCH_PACKAGE(/usr/local/mozilla /usr/local/mozilla/dist/*,,
	              nspr.h, nspr3, PR_Init, nspr)
  else
    AC_SEARCH_PACKAGE($with_nspr $with_nspr/dist/*,,
	              nspr.h, nspr3, PR_Init, nspr)
  fi

  if test "$with_nspr" != ""; then
    if test "$nspr_PKG" != "yes"; then
      dnl  If the user specified to search for NSPR but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate NSPR package.  Use --with-nspr=directory,"
      echo "     e.g. --with-nspr=/usr/local, or just --with-nspr."
      echo "     If NSPR is not installed, specify --without-nspr."
      echo ""
      exit 1
    fi
  fi

  if test "$nspr_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_NSPR)
#endif
    have_nspr=yes
    nspr_cflags="-I$nspr_INCLUDE"
    nspr_ldflags="-L$nspr_LIB"
    nspr_libs="-lnspr3"
  fi
fi

AC_SUBST(have_nspr)
AC_SUBST(nspr_cflags)
AC_SUBST(nspr_lflags)
AC_SUBST(nspr_libs)
AM_CONDITIONAL(HAVE_NSPR, test "$have_nspr" = "yes")


AC_ARG_WITH(zlib,
[  --with-zlib=DIR         Prefix where zlib is installed (usually /usr)])

have_zlib=no
include_zlib=
if test "$with_zlib" != "no"; then
  if test "$with_zlib" = "yes" -o "$with_zlib" = ""; then
    AC_SEARCH_PACKAGE(/usr /usr/local,,
	              zlib.h, z, gzopen, zlib)
  else
    AC_SEARCH_PACKAGE($with_zlib,,
	              zlib.h, z, gzopen, zlib)
  fi

  if test "$with_zlib" != ""; then
    if test "$zlib_PKG" != "yes"; then
      dnl  If the user specified to search for zlib but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate zlib package.  Use --with-zlib=directory,"
      echo "     e.g. --with-zlib=/usr/local, or just --with-zlib."
      echo "     If zlib is not installed, specify --without-zlib."
      echo ""
      exit 1
    fi
  fi

  if test "$zlib_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_ZLIB)
#endif
    have_zlib=yes
    if test "$zlib_INCLUDE" != ""; then
      zlib_cflags="-I$zlib_INCLUDE"
    fi
    if test "$zlib_LIB" != ""; then
      zlib_lflags="-L$zlib_LIB"
    fi
    zlib_libs="-lz"
  fi
fi

AC_SUBST(have_zlib)
AC_SUBST(zlib_cflags)
AC_SUBST(zlib_lflags)
AC_SUBST(zlib_libs)
AM_CONDITIONAL(HAVE_ZLIB, test "$have_zlib" = "yes")


AC_ARG_WITH(gl,
[  --with-gl=DIR           Prefix where OpenGL is installed (usually /usr)])

have_gl=no
include_gl=
if test "$with_gl" != "no"; then
  if test "$with_gl" = "yes" -o "$with_gl" = ""; then
    AC_SEARCH_PACKAGE(/usr /usr/local,,
	              GL/gl.h, GL, glVertex3f, gl)
  else
    AC_SEARCH_PACKAGE($with_gl,,
	              GL/gl.h, GL, glVertex3f, gl)
  fi

  if test "$with_gl" != ""; then
    if test "$gl_PKG" != "yes"; then
      dnl  If the user specified to search for OpenGL but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate OpenGL package.  Use --with-gl=directory,"
      echo "     e.g. --with-gl=/usr/local, or just --with-gl."
      echo "     If OpenGL is not installed, specify --without-gl."
      echo ""
      exit 1
    fi
  fi

  if test "$gl_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_GL)
#endif
    have_gl=yes
    if test "$gl_INCLUDE" != ""; then
      gl_cflags="-I$gl_INCLUDE"
    fi
    if test "$gl_LIB" != ""; then
      gl_lflags="-L$gl_LIB"
    fi
    gl_libs="-lGL -lGLU"
  fi
fi

AC_SUBST(have_gl)
AC_SUBST(gl_cflags)
AC_SUBST(gl_lflags)
AC_SUBST(gl_libs)
AM_CONDITIONAL(HAVE_GL, test "$have_gl" = "yes")


AC_ARG_WITH(glu,
[  --with-glu=DIR           Prefix where GL util library is installed (usually /usr)])

have_glu=no
include_glu=
if test "$with_glu" != "no"; then
  if test "$with_glu" = "yes" -o "$with_glu" = ""; then
    AC_SEARCH_PACKAGE($gl_INCLUDE /usr /usr/local,,
	              GL/glu.h, GLU, gluSphere, glu)
  else
    AC_SEARCH_PACKAGE($with_glu,,
	              GL/glu.h, GLU, gluSphere, glu)
  fi

  if test "$with_glu" != ""; then
    if test "$glu_PKG" != "yes"; then
      dnl  If the user specified to search for GL util library but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate GL util library.  Use --with-glu=directory,"
      echo "     e.g. --with-glu=/usr/local, or just --with-glu."
      echo "     If GL util library is not installed, specify --without-glu."
      echo ""
      exit 1
    fi
  fi

  if test "$glu_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_GLU)
#endif
    have_glu=yes
    if test "$glu_INCLUDE" != ""; then
      glu_cflags="-I$glu_INCLUDE"
    fi
    if test "$glu_LIB" != ""; then
      glu_lflags="-L$glu_LIB"
    fi
    glu_libs="-lGLU -lGLUU"
  fi
fi

AC_SUBST(have_glu)
AC_SUBST(glu_cflags)
AC_SUBST(glu_lflags)
AC_SUBST(glu_libs)
AM_CONDITIONAL(HAVE_GLU, test "$have_glu" = "yes")


AC_ARG_WITH(glx,
[  --with-glx=DIR          Prefix where GLX is installed (usually /usr)])

have_glx=no
include_glx=
if test "$with_glx" != "no"; then
  if test "$with_glx" = "yes" -o "$with_glx" = ""; then
    AC_SEARCH_HPACKAGE($gl_INCLUDE /usr /usr/local $x_libraries,,
	               GL/glx.h, glx)
  else
    AC_SEARCH_HPACKAGE($with_glx,,
	               GL/glx.h, glx)
  fi

  if test "$with_glx" != ""; then
    if test "$glx_PKG" != "yes"; then
      dnl  If the user specified to search for GLX but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate GLX package.  Use --with-glx=directory,"
      echo "     e.g. --with-glx=/usr/local, or just --with-glx."
      echo "     If GLX is not installed, specify --without-glx."
      echo ""
      exit 1
    fi
  fi

  if test "$glx_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_GLX)
#endif
    have_glx=yes
    if test "$glx_INCLUDE" != ""; then
      glx_cflags="-I$glx_INCLUDE"
    fi
  fi
fi

AC_SUBST(have_glx)
AC_SUBST(glx_cflags)
AM_CONDITIONAL(HAVE_GLX, test "$have_glx" = "yes")


AC_ARG_WITH(glut,
[  --with-glut=DIR         Prefix where glut is installed (usually /usr)])

have_glut=no
include_glut=
if test "$with_glut" != "no"; then
  if test "$with_glut" = "yes" -o "$with_glut" = ""; then
    AC_SEARCH_PACKAGE($gl_INCLUDE /usr /usr/local $x_libraries,,
	              GL/glut.h, glut, glutInit, glut, -lGL -lGLU)
  else
    AC_SEARCH_PACKAGE($with_glut,,
	              GLUT/glut.h, glut, glutInit, glut, -lGL -lGLU)
  fi

  if test "$with_glut" != ""; then
    if test "$glut_PKG" != "yes"; then
      dnl  If the user specified to search for glut but we didn't find it,
      dnl  abort now.

      echo ""
      echo "**** Could not locate glut package.  Use --with-glut=directory,"
      echo "     e.g. --with-glut=/usr/local, or just --with-glut."
      echo "     If glut is not installed, specify --without-glut."
      echo ""
      exit 1
    fi
  fi

  if test "$glut_PKG" = "yes"; then
#if $[ne $[CONFIG_HEADER],]
    AC_DEFINE(HAVE_GLUT)
#endif
    have_glut=yes
    if test "$glut_INCLUDE" != ""; then
      glut_cflags="-I$glut_INCLUDE"
    fi
    if test "$glut_LIB" != ""; then
      glut_lflags="-L$glut_LIB"
    fi
    glut_libs="-lglut"
  fi
fi

AC_SUBST(have_glut)
AC_SUBST(glut_cflags)
AC_SUBST(glut_lflags)
AC_SUBST(glut_libs)
AM_CONDITIONAL(HAVE_GLUT, test "$have_glut" = "yes")


AC_ARG_WITH(rib,
[  --with-rib              Compile in the Renderman interface.])

have_rib=no
if test "$with_rib" = "yes"; then
  have_rib=yes
fi

AC_SUBST(have_rib)
#if $[ne $[CONFIG_HEADER],]
if test "$have_rib" = "yes"; then
AC_DEFINE(HAVE_RIB)
fi
#endif
AM_CONDITIONAL(HAVE_RIB, test "$have_rib" = "yes")



AC_ARG_WITH(mikmod,
[  --with-mikmod[=libmikmod-config]  Use the mikmod interface for audio.])

have_mikmod=no
include_mikmod=
if test "$with_mikmod" != "no"; then
  if test "$with_mikmod" = "" -o "$with_mikmod" = "yes"; then
    dnl search for the libmikmod-config program on the path.
    AC_CHECK_PROG(with_mikmod, libmikmod-config, libmikmod-config, "no")
  fi
fi

if test "$with_mikmod" != "no"; then
  have_mikmod=yes
  CFLAGS="$CFLAGS "`$with_mikmod --cflags`  
  CXXFLAGS="$CXXFLAGS "`$with_mikmod --cflags`  
  LDFLAGS="$LDFLAGS "`$with_mikmod --libs`  
#if $[ne $[CONFIG_HEADER],]
  AC_DEFINE(HAVE_MIKMOD)
#endif
fi

AC_SUBST(have_mikmod)
AM_CONDITIONAL(HAVE_MIKMOD, test "$have_mikmod" = "yes")

ldflags=$LDFLAGS
AC_SUBST(ldflags)

AC_OUTPUT([$[TREE:%=%/Makefile]])

#end configure.in

//////////////////////////////////////////////////////////////////////
#endif // DIR_TYPE

//
// Config.OSX.pp
//
// This file defines some custom config variables for the osx
// platform.  It makes some initial guesses about compiler features,
// etc.
//

// *******************************************************************
// NOTE: you should not attempt to copy this file verbatim as your own
// personal Config.pp file.  Instead, you should start with an empty
// Config.pp file, and add lines to it when you wish to override
// settings given in here.  In the normal ppremake system, this file
// will always be read first, and then your personal Config.pp file
// will be read later, which gives you a chance to override the
// default settings found in this file.  However, if you start by
// copying the entire file, it will be difficult to tell which
// settings you have customized, and it will be difficult to upgrade
// to a subsequent version of Panda.
// *******************************************************************

#define IS_OSX 1

// Compiler flags

#define CC gcc
#define CXX g++
#define C++FLAGS_GEN -ftemplate-depth-30

// Configure for universal binaries on OSX.
#defer ARCH_FLAGS $[if $[UNIVERSAL_BINARIES],-arch i386 -arch ppc -arch x86_64,]
#define OSX_CDEFS
#define OSX_CFLAGS -Wno-deprecated-declarations

// Whether to build for Cocoa, Carbon or both.  64-bits systems do not
// have Carbon.  We also disable it for universal and 64-bits builds.
#define HAVE_COCOA 1
#defer HAVE_CARBON $[not $[or $[eq $[shell uname -m], x86_64],$[UNIVERSAL_BINARIES]]]

// How to compile a C or C++ file into a .o file.  $[target] is the
// name of the .o file, $[source] is the name of the source file,
// $[ipath] is a space-separated list of directories to search for
// include files, and $[flags] is a list of additional flags to pass
// to the compiler.

#defer COMPILE_C $[CC] $[CFLAGS_GEN] $[ARCH_FLAGS] $[OSX_CFLAGS] -c -o $[target] $[ipath:%=-I%] $[flags] $[source]
#defer COMPILE_C++ $[CXX] $[C++FLAGS_GEN] $[ARCH_FLAGS] $[OSX_CFLAGS] -c -o $[target] $[ipath:%=-I%] $[flags] $[source]

// What flags should be passed to both C and C++ compilers to enable
// debug symbols?  This will be supplied when OPTIMIZE (above) is set
// to 1, 2, or 3.
#defer DEBUGFLAGS -g

// What flags should be passed to both C and C++ compilers to enable
// compiler optimizations?  This will be supplied when OPTIMIZE
// (above) is set to 2, 3, or 4.
#defer OPTFLAGS -O2

// By convention, any source file that contains the string _no_opt_ in
// its filename won't have the above compiler optimizations run for it.
#defer no_opt $[findstring _no_opt_,$[source]]

// What define variables should be passed to the compilers for each
// value of OPTIMIZE?  We separate this so we can pass these same
// options to interrogate, guaranteeing that the correct interfaces
// are generated.  Do not include -D here; that will be supplied
// automatically.
#defer CDEFINES_OPT1 $[EXTRA_CDEFS] $[OSX_CDEFS]
#defer CDEFINES_OPT2 $[EXTRA_CDEFS] $[OSX_CDEFS]
#defer CDEFINES_OPT3 $[EXTRA_CDEFS] $[OSX_CDEFS]
#defer CDEFINES_OPT4 $[EXTRA_CDEFS] $[OSX_CDEFS]

// What additional flags should be passed for each value of OPTIMIZE
// (above)?  We separate out the compiler-optimization flags, above,
// so we can compile certain files that give optimizers trouble (like
// the output of lex and yacc) without them, but with all the other
// relevant flags.
#defer CFLAGS_OPT1 $[CDEFINES_OPT1:%=-D%] -Wall $[DEBUGFLAGS]
#defer CFLAGS_OPT2 $[CDEFINES_OPT2:%=-D%] -Wall $[DEBUGFLAGS] $[if $[no_opt],,$[OPTFLAGS]]
#defer CFLAGS_OPT3 $[CDEFINES_OPT3:%=-D%] $[DEBUGFLAGS] $[if $[no_opt],,$[OPTFLAGS]]
#defer CFLAGS_OPT4 $[CDEFINES_OPT4:%=-D%] $[if $[no_opt],,$[OPTFLAGS]]

// What additional flags should be passed to both compilers when
// building shared (relocatable) sources?  Some architectures require
// special support for this.
#defer CFLAGS_SHARED -fPIC

// How to generate a C or C++ executable from a collection of .o
// files.  $[target] is the name of the binary to generate, and
// $[sources] is the list of .o files.  $[libs] is a space-separated
// list of dependent libraries, and $[lpath] is a space-separated list
// of directories in which those libraries can be found.
#defer link_bin_opts $[ARCH_FLAGS] $[OSX_CFLAGS] \
 $[if $[not $[LINK_ALL_STATIC]],-undefined dynamic_lookup] \
 -o $[target] $[sources]\
 $[flags]\
 $[lpath:%=-L%] $[libs:%=-l%]\
 $[fpath:%=-Wl,-F%] $[patsubst %,-framework %, $[bin_frameworks]]

#defer LINK_BIN_C $[cc_ld] $[link_bin_opts]
#defer LINK_BIN_C++ $[cxx_ld] $[link_bin_opts]

// How to generate a static C or C++ library.  $[target] is the
// name of the library to generate, and $[sources] is the list of .o
// files that will go into the library.
#defer STATIC_LIB_C libtool -static -o $[target] $[sources]
#defer STATIC_LIB_C++ libtool -static -o $[target] $[sources]

// How to run ranlib, if necessary, after generating a static library.
// $[target] is the name of the library.  Set this to the empty string
// if ranlib is not necessary on your platform.
#defer RANLIB ranlib $[target]

// Where to put the so_locations file, used by an Irix MIPSPro
// compiler, to generate a map of shared library memory locations.
#defer SO_LOCATIONS $[DTOOL_INSTALL]/etc/so_locations


// How to generate a shared C or C++ library.  $[source] and $[target]
// as above, and $[libs] is a space-separated list of dependent
// libraries, and $[lpath] is a space-separated list of directories in
// which those libraries can be found.
#defer SHARED_LIB_C $[cc_ld] $[ARCH_FLAGS] $[OSX_CFLAGS] -o $[target] -dynamiclib -install_name $[notdir $[target]] $[sources] $[lpath:%=-L%] $[libs:%=-l%] $[patsubst %,-framework %, $[frameworks]]
#defer SHARED_LIB_C++ $[cxx_ld] $[ARCH_FLAGS] $[OSX_CFLAGS] -undefined dynamic_lookup -dynamic -dynamiclib -o $[target] -dynamiclib -install_name $[notdir $[target]] $[sources] $[lpath:%=-L%] $[libs:%=-l%] $[patsubst %,-framework %, $[frameworks]]
#defer BUNDLE_LIB_C++ $[cxx_ld] $[ARCH_FLAGS] $[OSX_CFLAGS] -undefined dynamic_lookup -bundle -o $[target] $[sources] $[lpath:%=-L%] $[libs:%=-l%] $[patsubst %,-framework %, $[frameworks]]

// How to install a data file or executable file.  $[local] is the
// local name of the file to install, and $[dest] is the name of the
// directory to put it in.

// On Unix systems, we strongly prefer using the install program to
// install files.  This has nice features like automatically setting
// the permissions bits, and also is usually clever enough to install
// a running program without crashing the running instance.  However,
// it doesn't understanding installing a program from a subdirectory,
// so we have to cd into the source directory first.
#defer install_dash_p $[if $[KEEP_TIMESTAMPS],-p,]
#defer INSTALL $[if $[ne $[dir $[local]], ./],cd ./$[dir $[local]] &&] install -m $[INSTALL_UMASK_DATA] $[install_dash_p] $[notdir $[local]] $[dest]/
#defer INSTALL_PROG $[if $[ne $[dir $[local]], ./],cd ./$[dir $[local]] &&] install -m $[INSTALL_UMASK_PROG] $[install_dash_p] $[notdir $[local]] $[dest]/


// Assume that OSX has OpenGL available.
#define HAVE_GL 1

// What additional flags should we pass to interrogate?
#define BASE_IGATE_FLAGS -D__FLT_EVAL_METHOD__=0  -D__const=const -Dvolatile -Dmutable -D__LITTLE_ENDIAN__ -D__inline__=inline -D__GNUC__
#define IGATE_ARCH -D__i386__
#defer SYSTEM_IGATE_FLAGS $[BASE_IGATE_FLAGS] $[IGATE_ARCH]

// We don't need worry about defining WORDS_BIGENDIAN (and we
// shouldn't anyway, since ppc and intel are different).  We rely on
// dtoolbase.h to determine this at compilation time.
#define WORDS_BIGENDIAN 

// Does the C++ compiler support namespaces?
#define HAVE_NAMESPACE 1

// Does the C++ compiler support ios::binary?
#define HAVE_IOS_BINARY 1

// How about the typename keyword?
#define HAVE_TYPENAME 1

// Will the compiler avoid inserting extra bytes in structs between a
// base struct and its derived structs?  It is safe to define this
// false if you don't know, but if you know that you can get away with
// this you may gain a tiny performance gain by defining this true.
// If you define this true incorrectly, you will get lots of
// assertion failures on execution.
#define SIMPLE_STRUCT_POINTERS

// Does gettimeofday() take only one parameter?
#define GETTIMEOFDAY_ONE_PARAM

// Do we have getopt() and/or getopt_long_only() built into the
// system?
#define HAVE_GETOPT 1
#define HAVE_GETOPT_LONG_ONLY 

// Are the above getopt() functions defined in getopt.h, or somewhere else?
#define PHAVE_GETOPT_H 1

// Can we determine the terminal width by making an ioctl(TIOCGWINSZ) call?
#define IOCTL_TERMINAL_WIDTH 1

// Do the system headers define a "streamsize" typedef?  How about the
// ios::binary enumerated value?  And other ios typedef symbols like
// ios::openmode and ios::fmtflags?
#define HAVE_STREAMSIZE 1
#define HAVE_IOS_BINARY 1
#define HAVE_IOS_TYPEDEFS 1

// Can we safely call getenv() at static init time?
#define STATIC_INIT_GETENV 1

// Can we read the file /proc/self/* to determine our
// environment variables at static init time?
#define HAVE_PROC_SELF_EXE
#define HAVE_PROC_SELF_MAPS
#define HAVE_PROC_SELF_ENVIRON
#define HAVE_PROC_SELF_CMDLINE

// Do we have a global pair of argc/argv variables that we can read at
// static init time?  Should we prototype them?  What are they called?
#define HAVE_GLOBAL_ARGV
#define PROTOTYPE_GLOBAL_ARGV
#define GLOBAL_ARGV __Argv
#define GLOBAL_ARGC __Argc

// Should we include <iostream> or <iostream.h>?  Define PHAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define PHAVE_IOSTREAM 1

// Do we have a true stringstream class defined in <sstream>?
#define PHAVE_SSTREAM 1

// Does fstream::open() require a third parameter, specifying the
// umask?  Versions of gcc prior to 3.2 had this.
#define HAVE_OPEN_MASK

// Do we have the lockf() function available?
#define HAVE_LOCKF 1

// Do the compiler or system libraries define wchar_t for you?
#define HAVE_WCHAR_T 1

// Does <string> define the typedef wstring?  Most do, but for some
// reason, versions of gcc before 3.0 didn't do this.
#define HAVE_WSTRING 1

// Do we have <new>?
#define PHAVE_NEW 1

// Do we have <io.h>?
#define PHAVE_IO_H

// Do we have <malloc.h>?
#define PHAVE_MALLOC_H

// Do we have <alloca.h>?
#define PHAVE_ALLOCA_H 1

// Do we have <locale.h>?
#define PHAVE_LOCALE_H 1

// Do we have <string.h>?
#define PHAVE_STRING_H 1

// Do we have <stdlib.h>?
#define PHAVE_STDLIB_H 1

// Do we have <limits.h>?
#define PHAVE_LIMITS_H 1

// Do we have <minmax.h>?
#define PHAVE_MINMAX_H

// Do we have <sys/types.h>?
#define PHAVE_SYS_TYPES_H 1
#define PHAVE_SYS_TIME_H 1

// Do we have <unistd.h>?
#define PHAVE_UNISTD_H 1

// Do we have <utime.h>?
#define PHAVE_UTIME_H 1

// Do we have <dirent.h>?
#define PHAVE_DIRENT_H 1

// Do we have <glob.h> (and do we want to use it instead of dirent.h)?
#define PHAVE_GLOB_H 1

// Do we have <sys/soundcard.h> (and presumably a Linux-style audio
// interface)?
#define PHAVE_SYS_SOUNDCARD_H 1

// Do we have <ucontext.h> (and therefore makecontext() / swapcontext())?
#define PHAVE_UCONTEXT_H 1

// Do we have RTTI (and <typeinfo>)?
#define HAVE_RTTI 1

// Do we have <stdint.h>?
#define PHAVE_STDINT_H 1

// The dynamic library file extension (usually .so .dll or .dylib):
#define DYNAMIC_LIB_EXT .dylib
#define STATIC_LIB_EXT .a

// If you need to build .so files in addition to .dylibs, declare this
// too.  Python 2.4 on OSX 10.4 seems to require this (it won't import
// a .dylib file directly).
//#define BUNDLE_EXT .so

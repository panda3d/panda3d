//
// Config.FreeBSD.pp
//
// This file defines some custom config variables for the FreeBSD
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

#define IS_FREEBSD 1

// What additional flags should we pass to interrogate?
#define SYSTEM_IGATE_FLAGS -D__i386__ -D__const=const -D__LITTLE_ENDIAN__ -D__inline__=inline -D__GNUC__

// Is the platform big-endian (like an SGI workstation) or
// little-endian (like a PC)?  Define this to the empty string to
// indicate little-endian, or nonempty to indicate big-endian.
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

// Do we have a gettimeofday() function?
#define HAVE_GETTIMEOFDAY 1

// Does gettimeofday() take only one parameter?
#define GETTIMEOFDAY_ONE_PARAM

// Do we have getopt() and/or getopt_long_only() built into the
// system?
#define HAVE_GETOPT 1
#define HAVE_GETOPT_LONG_ONLY

// Are the above getopt() functions defined in getopt.h, or somewhere else?
#define HAVE_GETOPT_H 1

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

// Can we read the file /proc/self/environ to determine our
// environment variables at static init time?
#define HAVE_PROC_SELF_ENVIRON

// Do we have a global pair of argc/argv variables that we can read at
// static init time?  Should we prototype them?  What are they called?
#define HAVE_GLOBAL_ARGV
#define PROTOTYPE_GLOBAL_ARGV
#define GLOBAL_ARGV
#define GLOBAL_ARGC

// Can we read the file /proc/curproc/* to determine our
// command-line arguments at static init time?
#define HAVE_PROC_SELF_EXE
#define HAVE_PROC_SELF_MAPS
#define HAVE_PROC_SELF_ENVIRON
#define HAVE_PROC_SELF_CMDLINE
#define HAVE_PROC_CURPROC_FILE 1
#define HAVE_PROC_CURPROC_MAP 1
#define HAVE_PROC_CURPROC_CMDLINE 1

// Should we include <iostream> or <iostream.h>?  Define HAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define HAVE_IOSTREAM 1

// Do we have a true stringstream class defined in <sstream>?
#define HAVE_SSTREAM 1

// Does fstream::open() require a third parameter, specifying the
// umask?  Versions of gcc prior to 3.2 had this.
#define HAVE_OPEN_MASK

// Do the compiler or system libraries define wchar_t for you?
#define HAVE_WCHAR_T 1

// Does <string> define the typedef wstring?  Most do, but for some
// reason, versions of gcc before 3.0 didn't do this.
#define HAVE_WSTRING 1

// Do we have <new>?
#define HAVE_NEW 1

// Do we have <io.h>?
#define HAVE_IO_H

// Do we have <malloc.h>?
//#define HAVE_MALLOC_H
#define HAVE_SYS_MALLOC_H 1

// Do we have <alloca.h>?
#define HAVE_ALLOCA_H

// Do we have <locale.h>?
#define HAVE_LOCALE_H 1

// Do we have <minmax.h>?
#define HAVE_MINMAX_H

// Do we have <sys/types.h>?
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_TIME_H 1

// Do we have <unistd.h>?
#define HAVE_UNISTD_H 1

// Do we have <utime.h>?
#define HAVE_UTIME_H 1

// Do we have <dirent.h>?
#define HAVE_DIRENT_H 1

// Do we have <glob.h> (and do we want to use it instead of dirent.h)?
#define HAVE_GLOB_H 1

// Do we have <sys/soundcard.h> (and presumably a Linux-style audio
// interface)?
#define HAVE_SYS_SOUNDCARD_H 1

// Do we have <ucontext.h> (and therefore makecontext() / swapcontext())?
#define HAVE_UCONTEXT_H

// Do we have RTTI (and <typeinfo>)?
#define HAVE_RTTI 1

#define HAVE_POSIX_THREADS 1

// Modern versions of gcc do support the latest STL allocator
// definitions.
#define USE_STL_ALLOCATOR 1

// The dynamic library file extension (usually .so .dll or .dylib):
#define DYNAMIC_LIB_EXT .so

#define PYTHON_IPATH /usr/local/include/python2.4
#define PYTHON_LPATH /usr/local/lib/python2.4

#define SSL_IPATH /usr/local/openssl
//#define SSL_097 1

#define JPEG_IPATH /usr/local/include
#define JPEG_LPATH /usr/local/lib

#define PNG_IPATH /usr/local/include
#define PNG_LPATH /usr/local/lib

#define TIFF_IPATH /usr/local/include
#define TIFF_LPATH /usr/local/lib

#define CC ccache cc
#define CXX ccache c++

//#define LOCAL_INCS /usr/local/include
//#define LOCAL_LIBS /usr/local/lib
//#defer ipath $[ipath] /usr/local/include
//#defer lpath $[lpath] /usr/local/lib

#defer EXTRA_IPATH $[EXTRA_IPATH] /usr/local/include
#defer EXTRA_LPATH $[EXTRA_LPATH] /usr/local/lib
//#defer alt_ipath $[alt_ipath] /usr/local/include
//#defer alt_lpath $[alt_lpath] /usr/local/lib


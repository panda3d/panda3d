//
// Config.Irix.pp
//
// This file defines some custom config variables for the SGI/Irix
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

// What additional flags should we pass to interrogate?
#define SYSTEM_IGATE_FLAGS -D__mips__ -D__MIPSEB__ -D_LANGUAGE_C_PLUS_PLUS -D_MIPS_SZINT=32 -D_MIPS_SZLONG=32 -D_MIPS_SZPTR=32

// Is the platform big-endian (like an SGI workstation) or
// little-endian (like a PC)?  Define this to the empty string to
// indicate little-endian, or nonempty to indicate big-endian.
#define WORDS_BIGENDIAN 1

// Does the C++ compiler support namespaces?
#define HAVE_NAMESPACE 1

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
#define HAVE_STREAMSIZE
#define HAVE_IOS_BINARY
#define HAVE_IOS_TYPEDEFS

// Can we safely call getenv() at static init time?
#define STATIC_INIT_GETENV 1

// Can we read the file /proc/self/environ to determine our
// environment variables at static init time?
#define HAVE_PROC_SELF_ENVIRON

// Do we have a global pair of argc/argv variables that we can read at
// static init time?  Should we prototype them?  What are they called?
#define HAVE_GLOBAL_ARGV 1
#define PROTOTYPE_GLOBAL_ARGV 1
#define GLOBAL_ARGV __Argv
#define GLOBAL_ARGC __Argc

// Can we read the file /proc/self/cmdline to determine our
// command-line arguments at static init time?
#define HAVE_PROC_SELF_CMDLINE

// Should we include <iostream> or <iostream.h>?  Define PHAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define PHAVE_IOSTREAM

// Do we have a true stringstream class defined in <sstream>?
#define PHAVE_SSTREAM

// Does fstream::open() require a third parameter, specifying the
// umask?
#define HAVE_OPEN_MASK 1

// Do we have the lockf() function available?
#define HAVE_LOCKF 1

// Do the compiler or system libraries define wchar_t for you?
#define HAVE_WCHAR_T 1

// Does <string> define the typedef wstring?  Most do, but for some
// reason, versions of gcc before 3.0 didn't do this.
#define HAVE_WSTRING 1

// Do we have <new>?
#define PHAVE_NEW

// Do we have <io.h>?
#define PHAVE_IO_H

// Do we have <malloc.h>?
#define PHAVE_MALLOC_H 1

// Do we have <alloca.h>?
#define PHAVE_ALLOCA_H 1

// Do we have <locale.h>?
#define PHAVE_LOCALE_H 1

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

// Do we have <sys/soundcard.h> (and presumably a Linux-style audio
// interface)?
#define PHAVE_SYS_SOUNDCARD_H

// Do we have <stdint.h>?
#define PHAVE_STDINT_H

// Do we have RTTI (and <typeinfo>)?
#define HAVE_RTTI 1

// The Irix compiler doesn't support the modern STL allocator.
#define USE_STL_ALLOCATOR

// The dynamic library file extension (usually .so .dll or .dylib):
#define DYNAMIC_LIB_EXT .so
#define BUNDLE_EXT

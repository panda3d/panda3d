//
// Config.Irix.pp
//
// This file defines some custom config variables for the SGI/Irix
// platform.  It makes some initial guesses about compiler features,
// etc.
//

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

// Should we include <iostream> or <iostream.h>?  Define HAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define HAVE_IOSTREAM

// Do we have a true stringstream class defined in <sstream>?
#define HAVE_SSTREAM

// Does fstream::open() require a third parameter, specifying the
// umask?
#define HAVE_OPEN_MASK 1

// Does <string> define the typedef wstring?  Most do, but for some
// reason, versions of gcc before 3.0 didn't do this.
#define HAVE_WSTRING 1

// Do we have <new>?
#define HAVE_NEW

// Do we have <io.h>?
#define HAVE_IO_H

// Do we have <malloc.h>?
#define HAVE_MALLOC_H 1

// Do we have <alloca.h>?
#define HAVE_ALLOCA_H 1

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

// Do we have <sys/soundcard.h> (and presumably a Linux-style audio
// interface)?
#define HAVE_SYS_SOUNDCARD_H

// Do we have RTTI (and <typeinfo>)?
#define HAVE_RTTI 1

// Must global operator new and delete functions throw exceptions?
#define GLOBAL_OPERATOR_NEW_EXCEPTIONS

// What is the syntax of the STL allocator declaration?  See
// LocalSetup.pp for allowable values.
#define STL_ALLOCATOR OLD

//
// Config.Linux.pp
//
// This file defines some custom config variables for the Linux
// platform.  It makes some initial guesses about compiler features,
// etc.
//

// What additional flags should we pass to interrogate?
#define SYSTEM_IGATE_FLAGS -D__i386__

// Is the platform big-endian (like an SGI workstation) or
// little-endian (like a PC)?  Define this to the empty string to
// indicate little-endian, or nonempty to indicate big-endian.
#define WORDS_BIGENDIAN

// Does the C++ compiler support namespaces?
#define HAVE_NAMESPACE 1

// Does the C++ compiler support ios::binary?
#define HAVE_IOS_BINARY 1

// Do we have a gettimeofday() function?
#define HAVE_GETTIMEOFDAY 1

// Does gettimeofday() take only one parameter?
#define GETTIMEOFDAY_ONE_PARAM

// Do we have getopt() and/or getopt_long_only() built into the
// system?
#define HAVE_GETOPT 1
#define HAVE_GETOPT_LONG_ONLY 1

// Can we determine the terminal width by making an ioctl(TIOCGWINSZ) call?
#define IOCTL_TERMINAL_WIDTH 1

// Do the compiler or the system headers define a "streamsize" typedef?
#define HAVE_STREAMSIZE 1

// Can we safely call getenv() at static init time?
#define STATIC_INIT_GETENV

// Can we read the file /proc/self/environ to determine our
// environment variables at static init time?
#define HAVE_PROC_SELF_ENVIRON 1

// Do we have a global pair of argc/argv variables that we can read at
// static init time?  Should we prototype them?  What are they called?
#define HAVE_GLOBAL_ARGV
#define PROTOTYPE_GLOBAL_ARGV
#define GLOBAL_ARGV
#define GLOBAL_ARGC

// Can we read the file /proc/self/cmdline to determine our
// command-line arguments at static init time?
#define HAVE_PROC_SELF_CMDLINE 1

// Should we include <iostream> or <iostream.h>?  Define HAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define HAVE_IOSTREAM

// Do we have a true stringstream class defined in <sstream>?
#define HAVE_SSTREAM

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

// Do we have <unistd.h>?
#define HAVE_UNISTD_H 1

// Do we have <utime.h>?
#define HAVE_UTIME_H 1

// Do we have <dirent.h>?
#define HAVE_DIRENT_H 1

// Do we have <sys/soundcard.h> (and presumably a Linux-style audio
// interface)?
#define HAVE_SYS_SOUNDCARD_H 1

// Do we have RTTI (and <typeinfo>)?
#define HAVE_RTTI 1

// Must global operator new and delete functions throw exceptions?
#define GLOBAL_OPERATOR_NEW_EXCEPTIONS 1

// Do we expect the old gcc custom STL allocator?
#define GCC_STYLE_ALLOCATOR 1

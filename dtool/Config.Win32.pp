//
// Config.Win32.pp
//
// This file defines some custom config variables for the Windows
// platform.  It makes some initial guesses about compiler features,
// etc.
//

// Is the platform big-endian (like an SGI workstation) or
// little-endian (like a PC)?  Define this to the empty string to
// indicate little-endian, or nonempty to indicate big-endian.
#define WORDS_BIGENDIAN

// Does the C++ compiler support namespaces?
#define HAVE_NAMESPACE 1

// Does the C++ compiler support ios::binary?
#define HAVE_IOS_BINARY 1

// Do we have a gettimeofday() function?
#define HAVE_GETTIMEOFDAY

// Does gettimeofday() take only one parameter?
#define GETTIMEOFDAY_ONE_PARAM

// Do we have getopt() and/or getopt_long_only() built into the
// system?
#define HAVE_GETOPT
#define HAVE_GETOPT_LONG_ONLY

// Should we include <iostream> or <iostream.h>?  Define HAVE_IOSTREAM
// to nonempty if we should use <iostream>, or empty if we should use
// <iostream.h>.
#define HAVE_IOSTREAM 1

// Do we have a true stringstream class defined in <sstream>?
#define HAVE_SSTREAM 1

// Do we have <malloc.h>?
#define HAVE_MALLOC_H 1

// Do we have <alloca.h>?
#define HAVE_ALLOCA_H

// Do we have <sys/types.h>?
#define HAVE_SYS_TYPES_H 1

// Do we have <unistd.h>?
#define HAVE_UNISTD_H

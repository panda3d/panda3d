// This is the toplevel directory.  It contains configure.in and other
// stuff.

#define DIR_TYPE toplevel

#define SAMPLE_SOURCE_FILE src/pandabase/pandabase.cxx
#define REQUIRED_TREES dtool

#define EXTRA_DIST \
    Config.Irix.pp Config.Linux.pp Config.Win32.pp Package.pp

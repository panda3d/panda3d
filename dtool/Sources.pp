// This is the toplevel directory.  It contains configure.in and other
// stuff.

#define DIR_TYPE toplevel

#define CONFIG_HEADER dtool_config.h
#define SAMPLE_SOURCE_FILE src/dtoolbase/dtoolbase.cxx

#define EXTRA_DIST \
    Config.Irix.pp Config.Linux.pp Config.osx.pp Config.Win32.pp Config.Cygwin.pp \
    LocalSetup.pp Package.pp

//
// System.pp
//
// This is a system-wide configure file for ppremake.  It's normally
// #included from a package-specific Config.pp in the root of the
// source tree.  It makes variable declarations that are not normally
// user-editable, but are required to set up the normal processing of
// ppremake.
//


// Define DIR_TYPE as "src", since that's the most common kind of source
// file.
#if $[eq $[DIR_TYPE],]
  #define DIR_TYPE src
#endif

// Define where to look for the various kinds of system files.
#if $[eq $[DEPENDS_FILE],]
  #define DEPENDS_FILE $[PPREMAKE_DIR]/Depends.pp
#endif

#if $[eq $[GLOBAL_FILE],]
  #define GLOBAL_FILE $[PPREMAKE_DIR]/Global.pp
#endif

#if $[eq $[GLOBAL_TYPE_FILE],]
  #define GLOBAL_TYPE_FILE $[PPREMAKE_DIR]/Global.$[BUILD_TYPE].pp
#endif

#if $[eq $[TEMPLATE_FILE],]
  #define TEMPLATE_FILE $[PPREMAKE_DIR]/Template.$[BUILD_TYPE].pp
#endif

/* prc_parameters.h.  Generated automatically by CMake. */
/********************************** DO NOT EDIT ****************************/


/* The compiled-in default directory to look for the Configrc file, in
   the absence of the PRC_DIR environment variable set, and in
   the absence of anything specified via the configpath directive. */
#define DEFAULT_PRC_DIR "@DEFAULT_PRC_DIR@"

/* The compiled-in name of the environment variable(s) that contain
   the name of a single directory in which to search for prc files. */
#define PRC_DIR_ENVVARS "@PRC_DIR_ENVVARS@"

/* The compiled-in name of the environment variable(s) that contain
   the name of multiple directories, separated by DEFAULT_PATHSEP, in
   which to search for prc files. */
#define PRC_PATH_ENVVARS "@PRC_PATH_ENVVARS@"

/* This is a special variable that is rarely used; it exists primarily
   to support the Cygwin-based "ctattach" tools used by the Walt
   Disney VR Studio.  This defines a set of environment variable(s)
   that define a search path, as above; except that the directory
   names on these search paths are Panda-style filenames, not
   Windows-style filenames; and the path separator is always a space
   character, regardless of DEFAULT_PATHSEP. */
#define PRC_PATH2_ENVVARS "@PRC_PATH2_ENVVARS@"

/* The filename(s) to search for in the above paths.  Normally this is
   *.prc. */
#define PRC_PATTERNS "@PRC_PATTERNS@"

/* The filename(s) for encrypted prc files. */
#define PRC_ENCRYPTED_PATTERNS "@PRC_ENCRYPTED_PATTERNS@"

/* The encryption key used to decrypt any encrypted prc files
   identified by PRC_ENCRYPTED_PATTERNS. */
#define PRC_ENCRYPTION_KEY "@PRC_ENCRYPTION_KEY@"

/* The filename(s) to search for, and execute, in the above paths.
   Normally this is empty. */
#define PRC_EXECUTABLE_PATTERNS "@PRC_EXECUTABLE_PATTERNS@"

/* The environment variable that defines optional args to pass to
   executables found that match one of the above patterns. */
#define PRC_EXECUTABLE_ARGS_ENVVAR "@PRC_EXECUTABLE_ARGS_ENVVAR@"

/* Define if we want to enable the "trust_level" feature of prc config
   variables.  This requires OpenSSL and PRC_PUBLIC_KEYS_FILENAME,
   above. */
#cmakedefine PRC_RESPECT_TRUST_LEVEL

/* The trust level value for any legacy (DConfig) variables. */
#define PRC_DCONFIG_TRUST_LEVEL @PRC_DCONFIG_TRUST_LEVEL@

/* The amount by which we globally increment the trust level. */
#define PRC_INC_TRUST_LEVEL @PRC_INC_TRUST_LEVEL@

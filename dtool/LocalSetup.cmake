#
# LocalSetup.cmake
#
# This file contains further instructions to set up the DTOOL package
# when using CMake. In particular, it creates the dtool_config.h
# file based on the user's selected configure variables.
#

include(CheckCXXSourceCompiles)
include(CheckCSourceRuns)
include(CheckIncludeFileCXX)
include(CheckFunctionExists)
include(CheckTypeSize)
include(TestBigEndian)
include(TestForSTDNamespace)

# Check if this is a big-endian system.
test_big_endian(WORDS_BIGENDIAN)

# Define if we have lockf().
check_cxx_source_compiles("
#include <unistd.h>
int main(int argc, char *argv[]) { lockf(0, F_LOCK, 0); return 0; }
" PHAVE_LOCKF)

# Define if we can trust the compiler not to insert extra bytes in
# structs between base structs and derived structs.
check_c_source_runs("
struct A { int a; };
struct B : public A { int b; };
int main(int argc, char *argv[]) {
  struct B i;
  if ((size_t) &(i.b) == ((size_t) &(i.a)) + sizeof(struct A)) {
    return 0;
  } else {
    return 1;
  }
}" SIMPLE_STRUCT_POINTERS)

# Define if we have STL hash_map etc. available.
# We're not using this functionality at the moment, it seems.
set(HAVE_STL_HASH OFF)

# Check if we have a gettimeofday() function.
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)

# Define if gettimeofday() takes only one parameter.
check_cxx_source_compiles("
#include <sys/time.h>
int main(int argc, char *argv[]) {
  struct timeval tv;
  int result;
  result = gettimeofday(&tv);
  return 0;
}" GETTIMEOFDAY_ONE_PARAM)

# Check if we have getopt.
check_function_exists(getopt HAVE_GETOPT)
check_function_exists(getopt_long_only HAVE_GETOPT_LONG_ONLY)
check_include_file_cxx(getopt.h PHAVE_GETOPT_H)

# Define if you have ioctl(TIOCGWINSZ) to determine terminal width.
#XXX can we make a test case for this that isn't dependent on
# the current terminal?  It might also be useful for Cygwin users.
if(UNIX)
  set(IOCTL_TERMINAL_WIDTH 1)
endif()

# Do the system headers define key ios typedefs like ios::openmode
# and ios::fmtflags?
check_cxx_source_compiles("
#include <ios>
std::ios::openmode foo;
std::ios::fmtflags bar;
int main(int argc, char *argv[]) { return 0; }
" HAVE_IOS_TYPEDEFS)

# Define if the C++ iostream library defines ios::binary.
check_cxx_source_compiles("
#include <ios>
std::ios::openmode binary = std::ios::binary;
int main(int argc, char *argv[]) { return 0; }
" HAVE_IOS_BINARY)

# Can we safely call getenv() at static init time?
if(WIN32 OR UNIX)
  set(STATIC_INIT_GETENV 1)
endif()

# Can we read the file /proc/self/[*] to determine our
# environment variables at static init time?
if(IS_LINUX)
  set(HAVE_PROC_SELF_EXE 1)
  set(HAVE_PROC_SELF_MAPS 1)
  set(HAVE_PROC_SELF_ENVIRON 1)
  set(HAVE_PROC_SELF_CMDLINE 1)
endif()

if(IS_FREEBSD)
  set(HAVE_PROC_CURPROC_FILE 1)
  set(HAVE_PROC_CURPROC_MAP 1)
  set(HAVE_PROC_CURPROC_CMDLINE 1)
endif()

# Do we have a global pair of argc/argv variables that we can read at
# static init time? Should we prototype them? What are they called?
if(WIN32)
  set(HAVE_GLOBAL_ARGV ON)
  set(PROTOTYPE_GLOBAL_ARGV OFF)
  set(GLOBAL_ARGV __argv)
  set(GLOBAL_ARGC __argc)
endif()

# Do we have all these header files?
check_include_file_cxx(io.h PHAVE_IO_H)
check_include_file_cxx(iostream PHAVE_IOSTREAM)
check_include_file_cxx(malloc.h PHAVE_MALLOC_H)
check_include_file_cxx(sys/malloc.h PHAVE_SYS_MALLOC_H)
check_include_file_cxx(alloca.h PHAVE_ALLOCA_H)
check_include_file_cxx(locale.h PHAVE_LOCALE_H)
check_include_file_cxx(string.h PHAVE_STRING_H)
check_include_file_cxx(stdlib.h PHAVE_STDLIB_H)
check_include_file_cxx(limits.h PHAVE_LIMITS_H)
check_include_file_cxx(sstream PHAVE_SSTREAM)
check_include_file_cxx(new PHAVE_NEW)
check_include_file_cxx(sys/types.h PHAVE_SYS_TYPES_H)
check_include_file_cxx(sys/time.h PHAVE_SYS_TIME_H)
check_include_file_cxx(unistd.h PHAVE_UNISTD_H)
check_include_file_cxx(utime.h PHAVE_UTIME_H)
check_include_file_cxx(glob.h PHAVE_GLOB_H)
check_include_file_cxx(dirent.h PHAVE_DIRENT_H)
check_include_file_cxx(ucontext.h PHAVE_UCONTEXT_H) #TODO doesn't work on OSX, use sys/ucontext.h
check_include_file_cxx(linux/input.h PHAVE_LINUX_INPUT_H)
check_include_file_cxx(stdint.h PHAVE_STDINT_H)

# Do we have Posix threads?
#set(HAVE_POSIX_THREADS ${CMAKE_USE_PTHREADS_INIT})

# Do we have SSE2 support?
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-msse2 HAVE_SSE2)

# Set LINK_ALL_STATIC if we're building everything as static libraries.
# Also set the library type used for "modules" appropriately.
if(BUILD_SHARED_LIBS)
  set(LINK_ALL_STATIC OFF)
  set(MODULE_TYPE "MODULE"
    CACHE INTERNAL "" FORCE)

else()
  set(LINK_ALL_STATIC ON)
  set(MODULE_TYPE "STATIC"
    CACHE INTERNAL "" FORCE)

endif()

# Now go through all the packages and report whether we have them.
show_packages()

message("")
if(INTERROGATE_PYTHON_INTERFACE)
  message("Compilation will generate Python interfaces for Python ${PYTHON_VERSION_STRING}.")
else()
  message("Configuring Panda WITHOUT Python interfaces.")
endif()

if(HAVE_THREADS)
  if(SIMPLE_THREADS)
    message("Compilation will include simulated threading support.")
  elseif(DO_PIPELINING)
    message("Compilation will include full, pipelined threading support.")
  else()
    message("Compilation will include nonpipelined threading support.")
  endif()

else()
  message("Configuring Panda without threading support.")

endif()

message("")
message("See dtool_config.h for more details about the specified configuration.")

message("")

# Generate dtool_config.h
if(IS_MULTICONFIG)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    foreach(option ${PER_CONFIG_OPTIONS})
      # Check for the presence of a config-specific option, and override what's
      # in the cache if there is.
      if(DEFINED ${option}_${config})
        set(${option} ${${option}_${config}})
      endif()
    endforeach(option)

    # Generate a dtool_config.h for this specific config
    configure_file(dtool_config.h.in "${PROJECT_BINARY_DIR}/${config}/include/dtool_config.h")

    # unset() does not unset CACHE variables by default, just normal variables.
    # By doing this we're reverting back to what was in the cache.
    foreach(option ${PER_CONFIG_OPTIONS})
      unset(${option})
    endforeach(option)
  endforeach(config)
else()
  # Just configure things like normal.
  configure_file(dtool_config.h.in "${PROJECT_BINARY_DIR}/include/dtool_config.h")
endif()

install(FILES "${PANDA_OUTPUT_DIR}/include/dtool_config.h"
  COMPONENT CoreDevel
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

# Generate the package configuration file
export_packages("${PROJECT_BINARY_DIR}/Panda3DPackages.cmake")
install(FILES "${PROJECT_BINARY_DIR}/Panda3DPackages.cmake"
  COMPONENT CoreDevel
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Panda3D")

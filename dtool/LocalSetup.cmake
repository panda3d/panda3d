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

# Define if we have libjpeg installed.
check_include_file_cxx(jpegint.h PHAVE_JPEGINT_H)

# Check if this is a big-endian system.
test_big_endian(WORDS_BIGENDIAN)

# Check if the compiler supports namespaces.
set(HAVE_NAMESPACE ${CMAKE_STD_NAMESPACE})

# Define if fstream::open() accepts a third parameter for umask.
check_cxx_source_compiles("
#include <fstream>
std::fstream fs;
int main(int argc, char *argv[]) { fs.open(\"file\", std::fstream::out, 0644); return 0; }
" HAVE_OPEN_MASK)

# Define if we have lockf().
check_cxx_source_compiles("
#include <unistd.h>
int main(int argc, char *argv[]) { lockf(0, F_LOCK, 0); return 0; }
" HAVE_LOCKF)

# Check if we have a wchar_t type.
check_type_size(wchar_t HAVE_WCHAR_T)

# Check if we have a wstring type.
check_cxx_source_compiles("
#include <string>
std::wstring str;
int main(int argc, char *argv[]) { return 0; }
" HAVE_WSTRING)

# Define if the C++ compiler supports the typename keyword.
# Since we now require C++11, this is a given.
set(HAVE_TYPENAME 1)

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

# Do the system headers define a "streamsize" typedef?
check_cxx_source_compiles("
#include <ios>
std::streamsize ss;
int main(int argc, char *argv[]) { return 0; }
" HAVE_STREAMSIZE)

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
check_include_file_cxx(minmax.h PHAVE_MINMAX_H)
check_include_file_cxx(sstream PHAVE_SSTREAM)
check_include_file_cxx(new PHAVE_NEW)
check_include_file_cxx(sys/types.h PHAVE_SYS_TYPES_H)
check_include_file_cxx(sys/time.h PHAVE_SYS_TIME_H)
check_include_file_cxx(unistd.h PHAVE_UNISTD_H)
check_include_file_cxx(utime.h PHAVE_UTIME_H)
check_include_file_cxx(glob.h PHAVE_GLOB_H)
check_include_file_cxx(dirent.h PHAVE_DIRENT_H)
check_include_file_cxx(sys/soundcard.h PHAVE_SYS_SOUNDCARD_H)
check_include_file_cxx(ucontext.h PHAVE_UCONTEXT_H) #TODO doesn't work on OSX, use sys/ucontext.h
check_include_file_cxx(linux/input.h PHAVE_LINUX_INPUT_H)
check_include_file_cxx(stdint.h PHAVE_STDINT_H)
check_include_file_cxx(typeinfo HAVE_RTTI)

# Do we have Posix threads?
#set(HAVE_POSIX_THREADS ${CMAKE_USE_PTHREADS_INIT})

# Do we have SSE2 support?
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-msse2 HAVE_SSE2)

#/* Define if needed to have 64-bit file i/o */
#$[cdefine __USE_LARGEFILE64]

# Set LINK_ALL_STATIC if we're building everything as static libraries.
if(BUILD_SHARED_LIBS)
  set(LINK_ALL_STATIC OFF)
else()
  set(LINK_ALL_STATIC ON)
endif()

# Now go through all the packages and report whether we have them.
show_packages()

if(HAVE_SPEEDTREE)
  message("+ SpeedTree")
else()
  message("- Did not find SpeedTree")
endif()

if(HAVE_GTK2)
  message("+ gtk+-2")
else()
  message("- Did not find gtk+-2")
endif()

if(HAVE_WX)
  message("+ WxWidgets")
else()
  message("- Did not find WxWidgets")
endif()

if(HAVE_FLTK)
  message("+ FLTK")
else()
  message("- Did not find FLTK")
endif()

if(HAVE_GL)
  message("+ OpenGL")
else()
  message("- Did not find OpenGL")
endif()

if(HAVE_GLES)
  message("+ OpenGL ES 1")
else()
  message("- Did not find OpenGL ES 1")
endif()

if(HAVE_GLES2)
  message("+ OpenGL ES 2")
else()
  message("- Did not find OpenGL ES 2")
endif()

if(HAVE_DX9)
  message("+ DirectX9")
else()
  message("- Did not find DirectX9")
endif()

if(HAVE_TINYDISPLAY)
  message("+ Tinydisplay")
else()
  message("- Not building Tinydisplay")
endif()

if(HAVE_X11)
  message("+ X11")
else()
  message("- Did not find X11")
endif()

if(HAVE_OPENCV)
  message("+ OpenCV")
else()
  message("- Did not find OpenCV")
endif()

if(HAVE_AWESOMIUM)
  message("+ AWESOMIUM")
else()
  message("- Did not find AWESOMIUM")
endif()

if(HAVE_MAYA)
  message("+ OpenMaya")
else()
  message("- Did not find OpenMaya")
endif()

if(HAVE_FCOLLADA)
  message("+ FCollada")
else()
  message("- Did not find FCollada")
endif()

if(HAVE_ASSIMP)
  message("+ Assimp")
else()
  message("- Did not find Assimp")
endif()

if(HAVE_ARTOOLKIT)
  message("+ ARToolKit")
else()
  message("- Did not find ARToolKit")
endif()

if(HAVE_ROCKET)
  if(HAVE_ROCKET_PYTHON)
    message("+ libRocket with Python bindings")
  else()
    message("+ libRocket without Python bindings")
  endif()
else()
  message("- Did not find libRocket")
endif()

if(HAVE_VORBIS)
  message("+ libvorbis (Ogg Vorbis Decoder)")
else()
  message("- Did not find libvorbis (Ogg Vorbis Decoder)")
endif()

message("")
if(HAVE_INTERROGATE AND HAVE_PYTHON)
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
configure_file(dtool_config.h.in "${PROJECT_BINARY_DIR}/include/dtool_config.h")
include_directories("${PROJECT_BINARY_DIR}/include")
#install(FILES "${PROJECT_BINARY_DIR}/dtool_config.h" DESTINATION include/panda3d)

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
#TODO make test case
#$[cdefine HAVE_OPEN_MASK]

# Define if we have lockf().
#TODO make test case
set(HAVE_LOCKF 1)

# Check if we have a wchar_t type.
check_type_size(wchar_t HAVE_WCHAR_T)

# Check if we have a wstring type.
check_cxx_source_compiles("
#include <string>
std::wstring str;
int main(int argc, char *argv[]) { return 0; }
" HAVE_WSTRING)

# Define if the C++ compiler supports the typename keyword.
#TODO make test case (I had one but it broke)
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
#TODO make test case
set(HAVE_IOS_TYPEDEFS 1)

# Define if the C++ iostream library defines ios::binary.
#TODO make test case
#$[cdefine HAVE_IOS_BINARY]

# Can we safely call getenv() at static init time?
#TODO make test case? can we make a reliable one?
#$[cdefine STATIC_INIT_GETENV]

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
#TODO make test case
#$[cdefine HAVE_GLOBAL_ARGV]
#$[cdefine PROTOTYPE_GLOBAL_ARGV]
#$[cdefine GLOBAL_ARGV]
#$[cdefine GLOBAL_ARGC]

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
check_include_file_cxx(drfftw.h PHAVE_DRFFTW_H)
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
message("")
message("Configuring support for the following optional third-party packages:")
if(HAVE_EIGEN)
  message("+ Eigen linear algebra library")
  if(LINMATH_ALIGN)
    message("+   (vectorization enabled in build)")
  else()
    message("-   (vectorization NOT enabled in build)")
  endif()
else()
  message("- Did not find Eigen linear algebra library")
endif()

if(HAVE_OPENSSL)
  message("+ OpenSSL")
else()
  message("- Did not find OpenSSL")
endif()

if(HAVE_JPEG)
  message("+ libjpeg")
else()
  message("- Did not find libjpeg")
endif()

if(HAVE_PNG)
  message("+ libpng")
else()
  message("- Did not find libpng")
endif()

if(HAVE_TIFF)
  message("+ libtiff")
else()
  message("- Did not find libtiff")
endif()

if(HAVE_TAR)
  message("+ libtar")
else()
  message("- Did not find libtar")
endif()

if(HAVE_FFTW)
  message("+ fftw")
else()
  message("- Did not find fftw")
endif()

if(HAVE_SQUISH)
  message("+ squish")
else()
  message("- Did not find squish")
endif()

if(HAVE_CG)
  message("+ Nvidia Cg High Level Shading Language")
else()
  message("- Did not find Nvidia Cg High Level Shading Language")
endif()

if(HAVE_CGGL)
  message("+ Cg OpenGL API")
else()
  message("- Did not find Cg OpenGL API")
endif()

if(HAVE_CGDX8)
  message("+ Cg DX8 API")
else()
  message("- Did not find Cg DX8 API")
endif()

if(HAVE_CGDX9)
  message("+ Cg DX9 API")
else()
  message("- Did not find Cg DX9 API")
endif()

if(HAVE_CGDX10)
  message("+ Cg DX10 API")
else()
  message("- Did not find Cg DX10 API")
endif()

if(HAVE_VRPN)
  message("+ VRPN")
else()
  message("- Did not find VRPN")
endif()

if(HAVE_ZLIB)
  message("+ zlib")
else()
  message("- Did not find zlib")
endif()

if(HAVE_RAD_MSS)
  message("+ Miles Sound System")
else()
  message("- Did not find Miles Sound System")
endif()

if(HAVE_FMODEX)
  message("+ FMOD Ex sound library")
else()
  message("- Did not find FMOD Ex sound library")
endif()

if(HAVE_OPENAL)
  message("+ OpenAL sound library")
else()
  message("- Did not find OpenAL sound library")
endif()

if(HAVE_PHYSX)
  message("+ Ageia PhysX")
else()
  message("- Did not find Ageia PhysX")
endif()

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

if(HAVE_FREETYPE)
  message("+ Freetype")
else()
  message("- Did not find Freetype")
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

if(HAVE_DX8)
  message("+ DirectX8")
else()
  message("- Did not find DirectX8")
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

if(HAVE_MESA)
  message("+ Mesa")
else()
  message("- Did not find Mesa")
endif()

if(HAVE_OPENCV)
  message("+ OpenCV")
else()
  message("- Did not find OpenCV")
endif()

if(HAVE_FFMPEG)
  message("+ FFMPEG")
else()
  message("- Did not find FFMPEG")
endif()

if(HAVE_ODE)
  message("+ ODE")
else()
  message("- Did not find ODE")
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

if(HAVE_BULLET)
  message("+ Bullet Physics")
else()
  message("- Did not find Bullet Physics")
endif()

if(HAVE_VORBIS)
  message("+ libvorbis (Ogg Vorbis Decoder)")
else()
  message("- Did not find libvorbis (Ogg Vorbis Decoder)")
endif()

message("")
if(HAVE_INTERROGATE AND HAVE_PYTHON)
  message("Compilation will generate Python interfaces.")
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

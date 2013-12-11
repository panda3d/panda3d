message(STATUS "")
message("Configuring support for the following optional third-party packages:")

# Settings to change USE_PACKAGE behavior (these options override cached values)
set(CONFIG_ENABLE_EVERYTHING Off)
set(CONFIG_DISABLE_EVERYTHING Off)
set(CONFIG_ENABLE_FOUND Off)
set(CONFIG_DISABLE_MISSING Off)

# Update USE_PACKAGE settings based on CLI arguments
if(EVERYTHING)
	message("Re-enabling all disabled third-party libraries.")
	set(CONFIG_ENABLE_EVERYTHING On)
elseif(DISCOVERED)
	message("Enabling found and disabling not-found third-party libraries.")
	set(CONFIG_ENABLE_FOUND On)
	set(CONFIG_DISABLE_MISSING On)
elseif(NOTHING)
	message("Disabling all third-party libraries.")
	set(CONFIG_DISABLE_EVERYTHING On)
endif()

include(ConfigurePackage)

# Find and configure Eigen library
find_package(Eigen3)
config_package(EIGEN COMMENT "eigen")
if(HAVE_EIGEN)
	if(WIN32)
		set(EIGEN_CFLAGS "/arch:SSE2")
	else()
		set(EIGEN_CFLAGS "-msse2")
	endif()

	if(CONFIG_ENABLE_EVERYTHING OR CONFIG_ENABLE_FOUND)
		unset(BUILD_EIGEN_VECTORIZATION)
	elseif(CONFIG_DISABLE_EVERYTHING)
		set(BUILD_EIGEN_VECTORIZATION OFF CACHE BOOL "If on, vectorization is enabled in build.")
	endif()

	if(NOT DEFINED BUILD_EIGEN_VECTORIZATION)
		message(STATUS "+   (vectorization enabled in build)")
	endif()

	set(BUILD_EIGEN_VECTORIZATION ON CACHE BOOL "If on, vectorization is enabled in build.")

	if(BUILD_EIGEN_VECTORIZATION)
		set(LINMATH_ALIGN TRUE)
	endif()
else()
	unset(BUILD_EIGEN_VECTORIZATION CACHE)
endif()

# Find and configure OpenSSL library
find_package(OpenSSL QUIET COMPONENTS ssl crypto)
mangle_package(OpenSSL)
config_package(OPENSSL COMMENT "OpenSSL")
if(HAVE_OPENSSL)
	if(CONFIG_ENABLE_EVERYTHING OR CONFIG_ENABLE_FOUND)
		unset(BUILD_OPENSSL_ERROR_REPORTS)
	endif()

	if(NOT DEFINED OPTIMIZE OR OPTIMIZE LESS 4)
		set(BUILD_OPENSSL_ERROR_REPORTS ON CACHE BOOL "If on, OpenSSL reports verbose error messages when they occur.")
	else()
		set(BUILD_OPENSSL_ERROR_REPORTS OFF CACHE BOOL "If on, OpenSSL reports verbose error messages when they occur.")
	endif()

	if(BUILD_OPENSSL_ERROR_REPORTS)
		set(REPORT_OPENSSL_ERRORS TRUE)
	endif()
else()
	unset(BUILD_OPENSSL_ERROR_REPORTS CACHE)
endif()

# Find and configure JPEG library
find_package(JPEG QUIET COMPONENTS jpeg)
mangle_package(JPEG)
config_package(JPEG COMMENT "libjpeg")

# Find and configure PNG library
find_package(PNG QUIET COMPONENTS png)
mangle_package(PNG)
config_package(PNG COMMENT "libpng")

# Find and configure TIFF library
find_package(TIFF QUIET COMPONENTS tiff z)
mangle_package(TIFF)
config_package(TIFF COMMENT "libtiff")

# Find and configure Tar library
find_package(Tar)
config_package(TAR COMMENT "libtar")

# Find and configure FFTW library
find_package(FFTW)
config_package(FFTW COMMENT "fftw")

# Find and configure Squish library
find_package(Squish)
config_package(SQUISH COMMENT "squish")

# Find and configure Cg library
find_package(Cg)
config_package(CG COMMENT "Nvidia Cg Shading Langauge")
config_package(CGGL COMMENT "Cg OpenGL API")
config_package(CGDX8 COMMENT "Cg DX8 API")
config_package(CGDX9 COMMENT "Cg DX9 API")
config_package(CGDX10 COMMENT "Cg DX10 API")

# Find and configure VRPN library
find_package(VRPN)
config_package(VRPN)

# Find and configure zlib
find_package(ZLIB QUIET COMPONENTS z)
mangle_package(ZLIB)
config_package(ZLIB COMMENT "zlib")

# Find and configure Miles Sound System
find_package(Miles)
config_package(RAD_MSS COMMENT "Miles Sound System")

# Find and configure FMOD Ex
find_package(FMODEx)
config_package(FMODEX COMMENT "FMOD Ex sound library")

# Find and configure OpenAL
find_package(OpenAL QUIET)
mangle_package(OpenAL)
config_package(OPENAL COMMENT "OpenAL sound library")

# Find and configure GTK
find_package(GTK2 QUIET)
mangle_package(GTK2)
config_package(GTK COMMENT "gtk+-2")

# Find and configure Freetype
find_package(Freetype)
mangle_package(Freetype)
config_package(FREETYPE COMMENT "Freetype")
if(HAVE_FREETYPE AND NOT WIN32)
	set(FREETYPE_CONFIG freetype-config)
endif()


########
# TODO #
########

# Find and configure PhysX
#find_package(PhysX)
#config_package(PHYSX COMMENT "Aegia PhysX")

# Find and configure SpeedTree
#find_package(SpeedTree)
#config_package(SPEEDTREE COMMENT "SpeedTree")

# Find and configure WxWidgets
#find_package(WxWidgets)
#config_package(WX COMMENT "WxWidgets")

# Find and configure FLTK
#find_package(FLTK)
#config_package(FLTK)

# Find and configure OpenGL
#find_package(OpenGL)
#config_package(OPENGL COMMENT "OpenGL")

# Find and configure OpenGL ES 1
#find_package(GLES)
#config_package(GLES COMMENT "OpenGL ES 1")

# Find and configure OpenGL ES 2
#find_package(GLES)
#config_package(GLES COMMENT "OpenGL ES 2")

# Find and configure DirectX 8
#find_package(DX8)
#config_package(DX8 COMMENT "DirectX8")

# Find and configure DirectX 9
#find_package(DX9)
#config_package(DX9 COMMENT "DirectX9")

# Find and configure DirectX 11
#find_package(DX11)
#config_package(DX11 COMMENT "DirectX11")

# Find and configure Tinydisplay
#find_package(Tinydisplay)
#config_package(TINYDISPLAY COMMENT "Tinydisplay")

#### Was commented out in original 'Config.pp' not sure why
# Find and configure SDL
#find_package(SDL)
#config_package(SDL)

# Find and configure Mesa
#find_package(Mesa)
#config_package(MESA COMMENT "Mesa")

# Find and configure OpenCV
#find_package(OpenCV)
#config_package(OPENCV COMMENT "OpenCV")

# Find and configure FFMPEG
#find_package(FFMPEG)
#config_package(FFMPEG)

# Find and configure ODE
#find_package(ODE)
#config_package(ODE)

# Find and configure Awesomium
#find_package(Awesomium)
#config_package(AWESOMIUM COMMENT "Awesomium")

# Find and configure OpenMaya
#find_package(OpenMaya)
#config_package(MAYA COMMENT "OpenMaya")

# Find and configure FCollada
#find_package(FCollada)
#config_package(FCOLLADA COMMENT "FCollada")
#if(FOUND_COLLADA14DOM OR FOUND_COLLADA15DOM)
#	set(USE_COLLADA TRUE CACHE BOOL "If true, compile Panda3D with COLLADA DOM")
#	if(USE_COLLADA)
#		if(FOUND_COLLADA15DOM)
#			set(HAVE_COLLADA15DOM TRUE)
#		else()
#			set(HAVE_COLLADA14DOM TRUE)
#		endif()
#	endif()
#endif()

# Find and configure Assimp
#find_package(Assimp)
#config_package(ASSIMP COMMENT "Assimp")

# Find and configure ARToolKit
#find_package(ARToolKit)
#config_package(ARTOOLKIT COMMENT "ARToolKit")

# Find and configure libRocket
#find_package(Rocket)
#config_package(ROCKET COMMENT "libRocket")
#if(HAVE_ROCKET)
#	# Check for rocket python bindings
#	if(FOUND_ROCKET_PYTHON)
#		set(USE_ROCKET_PYTHON TRUE CACHE BOOL "If true, compile Panda3D with python bindings for libRocket")
#		if(USE_ROCKET_PYTHON)
#			set(HAVE_ROCKET_PYTHON TRUE)
#		endif()
#	endif()
#	if(HAVE_ROCKET_PYTHON)
#		message(STATUS "+ libRocket with Python bindings")
#	else()
#		message(STATUS "+ libRocket without Python bindings")
#	endif()
#endif()

# Find and configure Bullet
#find_package(Bullet)
#config_package(BULLET COMMENT "Bullet Physics")

# Find and configure Vorbis
#find_package(Vorbis)
#config_package(VORBIS COMMENT "Vorbis Ogg decoder")


### Configure interrogate ###
message(STATUS "") # simple line break
if(HAVE_PYTHON)
	set(USE_INTERROGATE TRUE CACHE BOOL "If true, Panda3D will generate python interfaces")
	if(USE_INTERROGATE)
		set(HAVE_INTERROGATE TRUE)
	endif()
endif()
if(HAVE_INTERROGATE)
	message(STATUS "Compilation will generate Python interfaces.")
else()
	message(STATUS "Configuring Panda without Python interfaces.")
endif()


### Configure threading support ###
# Add basic use flag for threading
set(USE_THREADS TRUE CACHE BOOL "If true, compile Panda3D with threading support.")
if(USE_THREADS)
	set(HAVE_THREADS TRUE)
else()
	unset(BUILD_SIMPLE_THREADS CACHE)
	unset(BUILD_OS_SIMPLE_THREADS CACHE)
endif()

# Configure debug threads
if(NOT DEFINED OPTIMIZE OR OPTIMIZE LESS 3)
	set(BUILD_DEBUG_THREADS ON CACHE BOOL "If on, enables debugging of thread and sync operations (i.e. mutexes, deadlocks)")
else()
	set(BUILD_DEBUG_THREADS OFF CACHE BOOL "If on, enables debugging of thread and sync operations (i.e. mutexes, deadlocks)")
endif()
if(BUILD_DEBUG_THREADS)
	set(DEBUG_THREADS TRUE)
endif()

# Add advanced threading configuration
if(HAVE_THREADS)
	set(BUILD_SIMPLE_THREADS FALSE CACHE BOOL "If true, compile with simulated threads.")
	if(BUILD_SIMPLE_THREADS)
		message(STATUS "Compilation will include simulated threading support.")
		set(BUILD_OS_SIMPLE_THREADS TRUE CACHE BOOL "If true, OS threading constructs will be used to perform context switches.")

		set(SIMPLE_THREADS TRUE)
		if(BUILD_OS_SIMPLE_THREADS)
			set(OS_SIMPLE_THREADS TRUE)
		endif()
	else()
		unset(BUILD_OS_SIMPLE_THREADS CACHE)

		set(BUILD_PIPELINING TRUE CACHE BOOL "If true, compile with pipelined rendering.")
		if(BUILD_PIPELINING)
			message(STATUS "Compilation will include full, pipelined threading support.")
		else()
			message(STATUS "Compilation will include nonpipelined threading support.")
		endif()
	endif()
else()
	message(STATUS "Configuring Panda without threading support.")
endif()

set(HAVE_POSIX_THREADS FALSE)
if(NOT WIN32)
	find_path(PTHREAD_IPATH
		NAMES "pthread.h"
		PATHS "/usr/include"
	)
	if(PTHREAD_IPATH)
		set(HAVE_POSIX_THREADS TRUE)
		set(THREAD_LIBS pthread)
		set(CMAKE_CXX_FLAGS "-pthread")
		set(CMAKE_CXX_FLAGS_DEBUG "-pthread")
		set(CMAKE_CXX_FLAGS_RELEASE "-pthread")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-pthread")
		set(CMAKE_CXX_FLAGS_MINSIZEREL "-pthread")

		mark_as_advanced(PTHREAD_IPATH)
	endif()
endif()


### Configure pipelining ###
if(NOT DEFINED BUILD_PIPELINING)
	if(NOT DEFINED OPTIMIZE OR OPTIMIZE LESS 2)
		set(BUILD_PIPELINING TRUE CACHE BOOL "If true, compile with pipelined rendering.")
	else()
		set(BUILD_PIPELINING FALSE CACHE BOOL "If true, compile with pipelined rendering.")
	endif()
endif()
if(BUILD_PIPELINING)
	set(DO_PIPELINING TRUE)
endif()

### Configure OS X options ###
if(OSX_PLATFORM)
	set(BUILD_UNIVERSIAL_BINARIES TRUE CACHE BOOL "If true, compiling will create universal OS X binaries.")
	if(BUILD_UNIVERSAL_BINARIES)
		message(STATUS "Compilation will create universal binaries.")
		set(UNIVERSAL_BINARIES TRUE)
	else()
		message(STATUS "Compilation will not create universal binaries.")
	endif()
endif()

message(STATUS "")
message(STATUS "See dtool_config.h for more details about the specified configuration.\n")

include(CheckIncludeFileCXX)
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
check_include_file_cxx(ucontext.h PHAVE_UCONTEXT_H)
check_include_file_cxx(linux/input.h PHAVE_LINUX_INPUT_H)
check_include_file_cxx(stdint.h PHAVE_STDINT_H)

set(HAVE_NAMESPACE ON)
set(HAVE_LOCKF ON)
set(HAVE_WCHAR_T ON)
set(HAVE_WSTRING ON)
set(HAVE_TYPENAME ON)
set(SIMPLE_STRUCT_POINTERS ON)
set(HAVE_STREAMSIZE ON)
set(HAVE_IOS_TYPEDEFS ON)

if(WIN32)
	set(DEFAULT_PATHSEP ";")
else()
	set(DEFAULT_PATHSEP ":")
endif()

configure_file(dtool_config.h.cmake ${CMAKE_BINARY_DIR}/include/dtool_config.h)
include_directories("${CMAKE_BINARY_DIR}/include")

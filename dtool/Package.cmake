message(STATUS "")
message(STATUS "Configuring support for the following optional third-party packages:")

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

# Find and configure Eigen library
find_package(Eigen3 QUIET)
config_package(EIGEN "Eigen")
if(HAVE_EIGEN)
  if(WIN32)
    set(EIGEN_CFLAGS "/arch:SSE2")
  else()
    set(EIGEN_CFLAGS "-msse2")
  endif()

  if(CONFIG_ENABLE_EVERYTHING OR CONFIG_ENABLE_FOUND)
    unset(BUILD_EIGEN_VECTORIZATION)
  elseif(CONFIG_DISABLE_EVERYTHING)
    option(BUILD_EIGEN_VECTORIZATION "If on, vectorization is enabled in build." OFF)
  endif()

  if(NOT DEFINED BUILD_EIGEN_VECTORIZATION)
    message(STATUS "+   (vectorization enabled in build)")
  endif()

  option(BUILD_EIGEN_VECTORIZATION "If on, vectorization is enabled in build." ON)

  if(BUILD_EIGEN_VECTORIZATION)
    set(LINMATH_ALIGN TRUE)
  endif()
else()
  unset(BUILD_EIGEN_VECTORIZATION CACHE)
endif()


# Find and configure OpenSSL library
find_package(OpenSSL QUIET COMPONENTS ssl crypto)
config_package(OPENSSL "OpenSSL")
if(HAVE_OPENSSL)
  if(CONFIG_ENABLE_EVERYTHING OR CONFIG_ENABLE_FOUND)
    unset(BUILD_OPENSSL_ERROR_REPORTS)
  endif()

  if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    option(BUILD_OPENSSL_ERROR_REPORTS "If on, OpenSSL reports verbose error messages when they occur." ON)
  else()
    set(BUILD_OPENSSL_ERROR_REPORTS "If on, OpenSSL reports verbose error messages when they occur." OFF)
  endif()

  if(BUILD_OPENSSL_ERROR_REPORTS)
    set(REPORT_OPENSSL_ERRORS TRUE)
  endif()
else()
  unset(BUILD_OPENSSL_ERROR_REPORTS CACHE)
endif()


# Find and configure JPEG library
find_package(JPEG QUIET)
config_package(JPEG "libjpeg")

# Find and configure PNG library
find_package(PNG QUIET)
config_package(PNG "libpng")

# Find and configure TIFF library
find_package(TIFF QUIET COMPONENTS tiff z)
config_package(TIFF "libtiff")

# Find and configure Tar library
find_package(Tar QUIET)
config_package(TAR "libtar")

# Find and configure FFTW library
find_package(FFTW QUIET)
config_package(FFTW "fftw")

# Find and configure Squish library
find_package(Squish QUIET)
config_package(SQUISH "squish")

# Find and configure Cg library
find_package(Cg QUIET)
config_package(CG "Nvidia Cg Shading Langauge")
config_package(CGGL "Cg OpenGL API")
config_package(CGDX8 "Cg DX8 API")
config_package(CGDX9 "Cg DX9 API")
config_package(CGDX10 "Cg DX10 API")

# Find and configure VRPN library
find_package(VRPN QUIET)
config_package(VRPN)

# Find and configure zlib
find_package(ZLIB QUIET)
config_package(ZLIB "zlib")

# Find and configure Miles Sound System
find_package(Miles QUIET)
config_package(RAD_MSS "Miles Sound System")

# Find and configure FMOD Ex
find_package(FMODEx QUIET)
config_package(FMODEX "FMOD Ex sound library")

# Find and configure OpenAL
find_package(OpenAL QUIET)
config_package(OPENAL "OpenAL sound library")

# Find and configure GTK
set(Freetype_FIND_QUIETLY TRUE) # Fix for builtin FindGTK2
set(GTK2_GTK_FIND_QUIETLY TRUE) # Fix for builtin FindGTK2
find_package(GTK2 QUIET COMPONENTS gtk)
set(GTK_FOUND ${GTK2_FOUND}) # Mangle for convenience
config_package(GTK "gtk+-2")

# Find and configure Freetype
find_package(Freetype QUIET)
config_package(FREETYPE "Freetype")
if(HAVE_FREETYPE AND NOT WIN32)
  set(FREETYPE_CONFIG freetype-config)
endif()


########
# TODO #
########

# Find and configure PhysX
#find_package(PhysX)
#config_package(PHYSX "Aegia PhysX")

# Find and configure SpeedTree
#find_package(SpeedTree)
#config_package(SPEEDTREE "SpeedTree")

# Find and configure WxWidgets
find_package(wxWidgets QUIET)
if(WXWIDGETS_FOUND)
  set(WX_FOUND TRUE) # Mangle for convenience
  mark_as_advanced(wxWidgets_CONFIG_EXECUTABLE) # Cleanup after builtin find
  mark_as_advanced(wxWidgets_wxrc_EXECUTABLE) # Cleanup after builtin find
endif()
config_package(WX "WxWidgets")

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
# set(USE_COLLADA TRUE CACHE BOOL "If true, compile Panda3D with COLLADA DOM")
# if(USE_COLLADA)
#   if(FOUND_COLLADA15DOM)
#     set(HAVE_COLLADA15DOM TRUE)
#   else()
#     set(HAVE_COLLADA14DOM TRUE)
#   endif()
# endif()
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
#if(HAVE_ROCKET AND HAVE_PYTHON)
# # Check for rocket python bindings
# if(FOUND_ROCKET_PYTHON)
#   option(USE_ROCKET_PYTHON "If on, compile Panda3D with python bindings for libRocket" ON)
#   if(USE_ROCKET_PYTHON)
#     set(HAVE_ROCKET_PYTHON TRUE)
#   endif()
#   else()
#       unset(USE_ROCKET_PYTHON CACHE)
# endif()
# if(HAVE_ROCKET_PYTHON)
#   message(STATUS "+ libRocket with Python bindings")
# else()
#   message(STATUS "+ libRocket without Python bindings")
# endif()
#else()
# unset(USE_ROCKET_PYTHON CACHE)
#endif()

# Find and configure Bullet
#find_package(Bullet)
#config_package(BULLET COMMENT "Bullet Physics")

# Find and configure Vorbis
#find_package(Vorbis)
#config_package(VORBIS COMMENT "Vorbis Ogg decoder")

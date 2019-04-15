#
# CompilerFlags.cmake
#
# This file sets up various compiler flags (warning levels, turning off options
# we don't need, etc.)  It must be included directly from the root
# CMakeLists.txt, due to its use of set(...)
#

include(CheckCXXCompilerFlag)

# Panda3D is now a C++11 project. Newer versions of CMake support this out of
# the box; for older versions we take a shot in the dark:
if(CMAKE_VERSION VERSION_LESS "3.1")
  check_cxx_compiler_flag("-std=gnu++11" COMPILER_SUPPORTS_CXX11)
  if(COMPILER_SUPPORTS_CXX11)
    string(APPEND CMAKE_CXX_FLAGS " -std=gnu++11")
  else()
    string(APPEND CMAKE_CXX_FLAGS " -std=gnu++0x")
  endif()

else()
  set(CMAKE_CXX_STANDARD 11)

endif()

# Set certain CMake flags we expect
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up the output directory structure, mimicking that of makepanda
set(CMAKE_BINARY_DIR "${CMAKE_BINARY_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(MODULE_DESTINATION "lib")

# Runtime code assumes that dynamic modules have a "lib" prefix; Windows
# assumes that debug libraries have a _d suffix.
set(CMAKE_SHARED_MODULE_PREFIX "lib")
if(WIN32)
  set(CMAKE_DEBUG_POSTFIX "_d")

  # Windows uses libfoo.lib for static libraries and foo.lib/dll for dynamic.
  set(CMAKE_STATIC_LIBRARY_PREFIX "lib")

  # On Windows, modules (DLLs) are located in bin; lib is just for .lib files
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
  if(BUILD_SHARED_LIBS)
    set(MODULE_DESTINATION "bin")
  endif()
endif()

# Set warning levels
if(MSVC)
  string(APPEND CMAKE_C_FLAGS " /W3")
  string(APPEND CMAKE_CXX_FLAGS " /W3")

else()
  string(APPEND CMAKE_C_FLAGS " -Wall")
  string(APPEND CMAKE_CXX_FLAGS " -Wall")

endif()

if(NOT "x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC")
  set(disable_flags "-Wno-unused-function -Wno-unused-parameter")
  string(APPEND CMAKE_C_FLAGS " ${disable_flags}")
  string(APPEND CMAKE_CXX_FLAGS " ${disable_flags} -Wno-reorder")
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " -Wno-unused-variable")
  string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -Wno-unused-variable")
  string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -Wno-unused-variable")

  if(MSVC)
    # Clang behaving as MSVC
    string(APPEND CMAKE_C_FLAGS " -Wno-unused-command-line-argument")
    set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-microsoft-template -Wno-unused-command-line-argument")
  endif()
endif()

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# CMake will often pass -rdynamic when linking executables as a convenience for
# projects that might forget when to use ENABLE_EXPORTS.  This is preposterous,
# since it prevents the linker from removing symbols unneeded by the executable
# and stops us from identifying cases where ENABLE_EXPORTS is needed.
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# As long as we're figuring out compiler flags, figure out the flags for
# turning C++ exception support on and off
if(MSVC)
  set(cxx_exceptions_on "/EHsc")
  set(cxx_exceptions_off "/D_HAS_EXCEPTIONS=0")

else()
  check_cxx_compiler_flag("-fno-exceptions" COMPILER_SUPPORTS_FEXCEPTIONS)
  if(COMPILER_SUPPORTS_FEXCEPTIONS)
    set(cxx_exceptions_on "-fexceptions")
    set(cxx_exceptions_off "-fno-exceptions")

  else()
    set(cxx_exceptions_on)
    set(cxx_exceptions_off)

  endif()

endif()

set(cxx_exceptions_property "$<BOOL:$<TARGET_PROPERTY:CXX_EXCEPTIONS>>")
add_compile_options(
  "$<${cxx_exceptions_property}:${cxx_exceptions_on}>"
  "$<$<NOT:${cxx_exceptions_property}>:${cxx_exceptions_off}>")

# We should use -fvisibility=hidden everywhere, as it makes sure we think
# about what symbols really should be exposed externally.  For more info, see:
# https://gcc.gnu.org/wiki/Visibility
if(NOT MSVC)
  check_cxx_compiler_flag("-fvisibility=hidden" COMPILER_SUPPORTS_FVISIBILITY_HIDDEN)
  if(COMPILER_SUPPORTS_FVISIBILITY_HIDDEN)
    add_compile_options("-fvisibility=hidden")
  endif()
endif()

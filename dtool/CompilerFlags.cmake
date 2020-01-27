#
# CompilerFlags.cmake
#
# This file sets up various compiler flags (warning levels, turning off options
# we don't need, etc.)  It must be included directly from the root
# CMakeLists.txt, due to its use of set(...)
#

include(CheckCXXCompilerFlag)

# These are flags for the custom configurations we add
# Standard
if(MSVC)
  set(CMAKE_C_FLAGS_STANDARD "/Ox")
  set(CMAKE_CXX_FLAGS_STANDARD "/Ox")
else()
  set(CMAKE_C_FLAGS_STANDARD "-O3")
  set(CMAKE_CXX_FLAGS_STANDARD "-O3")
endif()
set(CMAKE_SHARED_LINKER_FLAGS_STANDARD "")
set(CMAKE_MODULE_LINKER_FLAGS_STANDARD "")
set(CMAKE_EXE_LINKER_FLAGS_STANDARD "")

# Coverage (when we know how to support it)
if(CMAKE_CXX_COMPILER_ID MATCHES "(AppleClang|Clang)")
  set(CMAKE_C_FLAGS_COVERAGE
    "${CMAKE_C_FLAGS_DEBUG} -fprofile-instr-generate -fcoverage-mapping")
  set(CMAKE_CXX_FLAGS_COVERAGE
    "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-instr-generate -fcoverage-mapping")

  set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fprofile-instr-generate")
  set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE
    "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fprofile-instr-generate")
  set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-instr-generate")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GCC")
  set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG} --coverage")
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

  set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} --coverage")
  set(CMAKE_MODULE_LINKER_FLAGS_COVERAGE
    "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")

endif()

# Panda3D is now a C++11 project. Newer versions of CMake support this out of
# the box; for older versions we take a shot in the dark:
if(CMAKE_VERSION VERSION_LESS "3.1")
  check_cxx_compiler_flag("-std=gnu++11" COMPILER_SUPPORTS_CXX11)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
  endif()

else()
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

endif()

# Set certain CMake flags we expect
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Set up the output directory structure, mimicking that of makepanda
set(CMAKE_BINARY_DIR "${CMAKE_BINARY_DIR}/cmake")

if(CMAKE_CFG_INTDIR STREQUAL ".")
  # Single-configuration generator; output goes straight in the binary dir
  set(PANDA_OUTPUT_DIR "${PROJECT_BINARY_DIR}")

else()
  # Multi-configuration generator; add a per-configuration path prefix
  set(PANDA_OUTPUT_DIR "${PROJECT_BINARY_DIR}/$<CONFIG>")

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PANDA_OUTPUT_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PANDA_OUTPUT_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PANDA_OUTPUT_DIR}/lib")

set(MODULE_DESTINATION "${CMAKE_INSTALL_LIBDIR}")

# Runtime code assumes that dynamic modules have a "lib" prefix; Windows
# assumes that debug libraries have a _d suffix.
set(CMAKE_SHARED_MODULE_PREFIX "lib")
if(WIN32)
  set(CMAKE_DEBUG_POSTFIX "_d")

  # Windows uses libfoo.lib for static libraries and foo.lib/dll for dynamic.
  set(CMAKE_STATIC_LIBRARY_PREFIX "lib")

  # On Windows, modules (DLLs) are located in bin; lib is just for .lib files
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PANDA_OUTPUT_DIR}/bin")
  if(BUILD_SHARED_LIBS)
    set(MODULE_DESTINATION "bin")
  endif()
endif()

# Though not technically correct, we'll still want MODULE type libraries
# (used for display and audio plugins) to use a .dylib extension.
if(APPLE)
  set(CMAKE_SHARED_MODULE_SUFFIX ".dylib")
endif()

# We want the output structured like build/CONFIG/bin, not build/bin/CONFIG per
# the default for multi-configuration generators. In CMake 3.4+, it switches
# automatically if the *_OUTPUT_DIRECTORY property contains a generator
# expresssion, but as of this writing we support as early as CMake 3.0.2.
#
# So, let's just do this:
if(CMAKE_VERSION VERSION_LESS "3.4")
  foreach(_type RUNTIME ARCHIVE LIBRARY)
    foreach(_config ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${_config}" _config)
      set(CMAKE_${_type}_OUTPUT_DIRECTORY_${_config} "${CMAKE_${_type}_OUTPUT_DIRECTORY}")
    endforeach(_config)
  endforeach(_type)
endif()

# Set warning levels
if(MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
  set(CMAKE_CCXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")

else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  set(global_flags
    "-Wno-unused-function -Wno-unused-parameter -fno-strict-aliasing")
  set(release_flags "-Wno-unused-variable")

  if(NOT MSVC)
    # These flags upset Clang when it's in MSVC mode
    set(release_flags "${release_flags} -fno-stack-protector -ffast-math -fno-unsafe-math-optimizations")

    # Allow NaN to occur in the public SDK
    set(standard_flags "${release_flags} -fno-finite-math-only")

  else()
    set(standard_flags "${release_flags}")

  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${global_flags}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${global_flags} -Wno-reorder")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${release_flags}")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${release_flags}")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${release_flags}")
  set(CMAKE_CXX_FLAGS_STANDARD "${CMAKE_CXX_FLAGS_STANDARD} ${standard_flags}")

  if(MSVC)
    # Clang behaving as MSVC
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-line-argument")
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

  set(cxx_rtti_on "/GR")
  set(cxx_rtti_off "/GR-")

else()
  check_cxx_compiler_flag("-fno-exceptions" COMPILER_SUPPORTS_FEXCEPTIONS)
  if(COMPILER_SUPPORTS_FEXCEPTIONS)
    set(cxx_exceptions_on "-fexceptions")
    set(cxx_exceptions_off "-fno-exceptions")

  else()
    set(cxx_exceptions_on)
    set(cxx_exceptions_off)

  endif()

  check_cxx_compiler_flag("-fno-rtti" COMPILER_SUPPORTS_FRTTI)
  if(COMPILER_SUPPORTS_FRTTI)
    set(cxx_rtti_on "-frtti")
    set(cxx_rtti_off "-fno-rtti")

  else()
    set(cxx_rtti_on)
    set(cxx_rtti_off)

  endif()

endif()

set(cxx_exceptions_property "$<BOOL:$<TARGET_PROPERTY:CXX_EXCEPTIONS>>")
add_compile_options(
  "$<${cxx_exceptions_property}:${cxx_exceptions_on}>"
  "$<$<NOT:${cxx_exceptions_property}>:${cxx_exceptions_off}>")

set(cxx_rtti_property "$<BOOL:$<TARGET_PROPERTY:CXX_RTTI>>")
if(NOT ANDROID)
  # Normally, our Debug build defaults RTTI on. This is not the case on
  # Android, where we don't use it even on Debug, to save space.

  set(cxx_rtti_property "$<OR:$<CONFIG:Debug>,${cxx_rtti_property}>")
endif()
add_compile_options(
  "$<${cxx_rtti_property}:${cxx_rtti_on}>"
  "$<$<NOT:${cxx_rtti_property}>:${cxx_rtti_off}>")
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" APPEND PROPERTY
  COMPILE_DEFINITIONS "$<${cxx_rtti_property}:HAVE_RTTI>")

if(MSVC)
  set(msvc_bigobj_property "$<BOOL:$<TARGET_PROPERTY:MSVC_BIGOBJ>>")
  add_compile_options("$<${msvc_bigobj_property}:/bigobj>")
endif()

# We should use -fvisibility=hidden everywhere, as it makes sure we think
# about what symbols really should be exposed externally.  For more info, see:
# https://gcc.gnu.org/wiki/Visibility
if(NOT MSVC)
  check_cxx_compiler_flag("-fvisibility=hidden" COMPILER_SUPPORTS_FVISIBILITY_HIDDEN)
  if(COMPILER_SUPPORTS_FVISIBILITY_HIDDEN)
    add_compile_options("-fvisibility=hidden")
  endif()
endif()

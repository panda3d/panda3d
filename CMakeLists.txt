cmake_minimum_required(VERSION 3.13)
set(CMAKE_DISABLE_SOURCE_CHANGES ON) # Must go before project() below
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON) # Must go before project() below

if(POLICY CMP0091)
  # Needed for CMake to pass /MD flag properly with non-VC generators.
  cmake_policy(SET CMP0091 NEW)
endif()

# Determine whether we are using a multi-config generator.
get_property(IS_MULTICONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Set the default CMAKE_BUILD_TYPE before calling project().
if(IS_MULTICONFIG)
  message(STATUS "Using multi-configuration generator")
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Standard CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "Using default build type ${CMAKE_BUILD_TYPE}")
  else()
    message(STATUS "Using build type ${CMAKE_BUILD_TYPE}")
  endif()
endif()

# Set defaults for macOS, must be before project().
if(APPLE)
  # Needed for enable_language(OBJCXX)
  cmake_minimum_required(VERSION 3.16)

  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum macOS version to target")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

  if(CMAKE_VERSION VERSION_LESS "3.19" AND NOT CMAKE_OSX_SYSROOT)
    # Older CMake chose SDK based on deployment target, against Apple's recommendations.
    # However, we need to use the latest to be able to target arm64.
    if(IS_DIRECTORY "/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk")
      set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX11.1.sdk" CACHE STRING "")
    elseif(IS_DIRECTORY "/Library/Developer/CommandLineTools/SDKs/MacOSX11.0.sdk")
      set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX11.0.sdk" CACHE STRING "")
    endif()
  endif()
endif()

# Figure out the version
set(_s "[\\t ]*") # CMake doesn't support \s*
file(STRINGS "setup.cfg" _version REGEX "^version${_s}=${_s}")
string(REGEX REPLACE "^.*=${_s}" "" _version "${_version}")
project(Panda3D VERSION ${_version})
unset(_version)
unset(_s)

if(APPLE)
  # Allows separating out C++ flags from ObjC++ flags
  enable_language(OBJCXX)
endif()

# Determine the possible build types.  Must be *after* calling project().
set(_configs Standard Release RelWithDebInfo Debug MinSizeRel)
if(CMAKE_CXX_COMPILER_ID MATCHES "(AppleClang|Clang|GCC)")
  list(APPEND _configs Coverage)
endif()

if(IS_MULTICONFIG)
  set(CMAKE_CONFIGURATION_TYPES "${_configs}" CACHE STRING "" FORCE)
else()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${_configs})
endif()

enable_testing()

string(REPLACE "$(EFFECTIVE_PLATFORM_NAME)" "" PANDA_CFG_INTDIR "${CMAKE_CFG_INTDIR}")

# Add generic modules to cmake module path,
# and add Panda3D specific modules to cmake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros/")

# When using the Xcode generator, don't append the platform name to the
# intermediate configuration directory.
set_property(GLOBAL PROPERTY XCODE_EMIT_EFFECTIVE_PLATFORM_NAME OFF)

# Include modules builtin to CMake
include(GNUInstallDirs)     # Defines CMAKE_INSTALL_<dir> variables

# Include global modules needed for configure scripts
include(PackageConfig)      # Defines package_option
include(PerConfigOption)    # Defines per_config_option

# Configure Panda3D
include(dtool/CompilerFlags.cmake)
include(dtool/PandaVersion.cmake)
include(dtool/Package.cmake)
include(dtool/Config.cmake)

# Include global modules
include(AddBisonTarget)     # Defines add_bison_target function
include(AddFlexTarget)      # Defines add_flex_target function
include(BuildMetalib)       # Defines add_component_library AND add_metalib
include(CompositeSources)   # Defines composite_sources function
include(Python)             # Defines add_python_target AND install_python_package
include(Interrogate)        # Defines target_interrogate AND add_python_module
include(RunPzip)            # Defines run_pzip function
include(Versioning)         # Hooks 'add_library' to apply VERSION/SOVERSION

# Determine which trees to build.
option(BUILD_DTOOL "Build the dtool source tree." ON)
option(BUILD_PANDA "Build the panda source tree." ON)
option(BUILD_DIRECT "Build the direct source tree." ON)
option(BUILD_PANDATOOL "Build the pandatool source tree." ON)
option(BUILD_CONTRIB "Build the contrib source tree." ON)
option(BUILD_MODELS "Build/install the built-in models." ON)
option(BUILD_TOOLS "Build binary tools." ON)

# Include Panda3D packages
if(BUILD_DTOOL)
  add_subdirectory(dtool "${CMAKE_BINARY_DIR}/dtool")
endif()

if(BUILD_PANDA)
  add_subdirectory(panda "${CMAKE_BINARY_DIR}/panda")
endif()

if(BUILD_DIRECT)
  add_subdirectory(direct "${CMAKE_BINARY_DIR}/direct")
endif()

if(BUILD_PANDATOOL)
  add_subdirectory(pandatool "${CMAKE_BINARY_DIR}/pandatool")
endif()

if(BUILD_CONTRIB)
  add_subdirectory(contrib "${CMAKE_BINARY_DIR}/contrib")
endif()

if(BUILD_MODELS)
  run_pzip(models
    "${CMAKE_CURRENT_SOURCE_DIR}/models/"
    "${PROJECT_BINARY_DIR}/${PANDA_CFG_INTDIR}/models"
    *.egg)

  add_custom_command(TARGET models
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND}
                             -DSOURCE="${CMAKE_CURRENT_SOURCE_DIR}/models/audio/"
                             -DDESTINATION="${PANDA_OUTPUT_DIR}/models/audio"
                             -P ${PROJECT_SOURCE_DIR}/cmake/scripts/CopyPattern.cmake
                     COMMENT "Copying models/audio")

  add_custom_command(TARGET models
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND}
                             -DSOURCE="${CMAKE_CURRENT_SOURCE_DIR}/models/icons/"
                             -DDESTINATION="${PANDA_OUTPUT_DIR}/models/icons"
                             -P ${PROJECT_SOURCE_DIR}/cmake/scripts/CopyPattern.cmake
                     COMMENT "Copying models/icons")

  add_custom_command(TARGET models
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND}
                             -DSOURCE="${CMAKE_CURRENT_SOURCE_DIR}/models/maps/"
                             -DDESTINATION="${PANDA_OUTPUT_DIR}/models/maps"
                             -P ${PROJECT_SOURCE_DIR}/cmake/scripts/CopyPattern.cmake
                     COMMENT "Copying models/maps")

  install(DIRECTORY "${PANDA_OUTPUT_DIR}/models"
    COMPONENT Models DESTINATION ${CMAKE_INSTALL_DATADIR}/panda3d)
endif()

if(INTERROGATE_PYTHON_INTERFACE)
  # If we built the Python interface, run the test suite.  Note, we do NOT test
  # for pytest before adding this test.  If the user doesn't have pytest, we'd
  # like for the tests to fail.

  # In the Coverage configuration, we also require pytest-cov

  add_test(NAME pytest
    COMMAND "${PYTHON_EXECUTABLE}" -m pytest "${PROJECT_SOURCE_DIR}/tests"
    $<$<CONFIG:Coverage>:--cov=.>
    WORKING_DIRECTORY "${PANDA_OUTPUT_DIR}")
endif()

# Generate the Panda3DConfig.cmake file so find_package(Panda3D) works, and
# also register the build directory with CMake's package registry.

file(COPY "${PROJECT_SOURCE_DIR}/cmake/install/Panda3DConfig.cmake"
  DESTINATION "${PROJECT_BINARY_DIR}")
install(FILES "${PROJECT_SOURCE_DIR}/cmake/install/Panda3DConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Panda3D")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/Panda3DConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion)
install(FILES "${PROJECT_BINARY_DIR}/Panda3DConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Panda3D")

if(NOT CMAKE_CROSSCOMPILING)
  export(PACKAGE Panda3D)
endif()

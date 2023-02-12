
# This file defines the current version number for Panda. It is read
# by the top CMakeLists.txt, which puts it in the global namespace for
# all CMake scripts for Panda.

option(PANDA_OFFICIAL_VERSION
  "This variable will be defined to false in the Git repository, but
scripts that generate source tarballs and/or binary releases for
distribution, by checking out Panda from an official Git tag,
should explictly set this to true.  When false, it indicates that
the current version of Panda was checked out from Git, so it may
not be a complete representation of the indicated version."
  OFF)

set(PANDA_DISTRIBUTOR homebuilt CACHE STRING
  "This string is reported verbatim by PandaSystem::get_distributor().
It should be set by whoever provides a particular distribution of
Panda.  If you build your own Panda, leave this unchanged.")

set(PANDA_DIST_USE_LICENSES "BSD-3;BSD-2;MIT" CACHE STRING
  "This is a list of allowed licenses for 3rd-party packages to build
support for when performing a Distribution build of Panda3d.
Note: This only is checked for packages that CMake has declared with a
particular license. Some packages don't have a listed license because
they are almost always required/used, or because they are only used in
plugins that can be easily removed (eg. directx, ffmpeg, fmod, ...).")

mark_as_advanced(PANDA_VERSION PANDA_OFFICIAL_VERSION PANDA_DIST_USE_LICENSES)

# The version gets a "c" at the end if it's not an official one.
if(PANDA_OFFICIAL_VERSION)
  set(VERSION_SUFFIX "")
else()
  set(VERSION_SUFFIX "c")
endif()

set(PANDA_VERSION_STR "${PROJECT_VERSION}${VERSION_SUFFIX}")

# This symbol is used to enforce ABI incompatibility between
# major versions of Panda3D.
set(PANDA_VERSION_SYMBOL panda_version_${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR})

# The Panda version as a number, with three digits reserved
# for each component.
math(EXPR PANDA_NUMERIC_VERSION "${PROJECT_VERSION_MAJOR}*1000000 + ${PROJECT_VERSION_MINOR}*1000 + ${PROJECT_VERSION_PATCH}")

# If SOURCE_DATE_EPOCH is set, it affects PandaSystem::get_build_date()
if(DEFINED ENV{SOURCE_DATE_EPOCH})
  string(TIMESTAMP _build_date "%b %d %Y %H:%M:%S" UTC)

  # CMake doesn't support %e, replace leading zero in day with space
  string(REGEX REPLACE "^([a-zA-Z]+) 0" "\\1  " PANDA_BUILD_DATE_STR "${_build_date}")
  unset(_build_date)
endif()

# The Panda Git SHA1 refspec, for PandaSystem::get_git_commit()
find_package(Git QUIET)
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE PANDA_GIT_COMMIT_STR
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

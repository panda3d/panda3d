# This file defines the current version number for Panda. It is read
# by the top CMakeLists.txt, which puts it in the global namespace for
# all CMake scripts for Panda.

option(PANDA_OFFICIAL_VERSION
  "This variable will be defined to false in the CVS repository, but
scripts that generate source tarballs and/or binary releases for
distribution, by checking out Panda from an official CVS tag,
should explictly set this to true.  When false, it indicates that
the current version of Panda was checked out from CVS, so it may
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

set(PANDA_PACKAGE_VERSION CACHE STRING
  "This string is used to describe the Panda3D \"package\" associated
with this current build of Panda. It should increment with major
and minor version changes, but not sequence (or \"bugfix\") changes.
It should be unique for each unique distributor. The default is
the empty string, which means this build does not precisely match
any distributable Panda3D packages. If you are making a Panda3D
build which you will be using to produce a distributable Panda3D
package, you should set this string appropriately.")

set(P3D_PLUGIN_VERSION "1.0.4" CACHE STRING
  "We also define a version for the Panda3D plugin/runtime,
i.e. nppanda3d.dll, p3dactivex.ocx, and panda3d.exe. This is an
independent version number from PANDA_VERSION or
PANDA_PACKAGE_VERSION, because it is anticipated that this plugin
code, once settled, will need to be updated much less frequently
than Panda itself.")

set(P3D_COREAPI_VERSION "${P3D_PLUGIN_VERSION}.1" CACHE STRING
  "Finally, there's a separate version number for the Core API. At
first, we didn't believe we needed a Core API version number, but
in this belief we were naive.  This version number is a little less
strict in its format requirements than P3D_PLUGIN_VERSION, above,
and it doesn't necessarily consist of a specific number of
integers, but by convention it will consist of four integers, with
the first three matching the plugin version, and the fourth integer
being incremented with each new Core API revision.")

mark_as_advanced(PANDA_VERSION PANDA_OFFICIAL_VERSION
  PANDA_PACKAGE_VERSION P3D_PLUGIN_VERSION P3D_COREAPI_VERSION
  PANDA_DIST_USE_LICENSES)

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

# The Panda Git SHA1 refspec, for PandaSystem::get_git_commit()
find_package(Git QUIET)
if(GIT_EXECUTABLE)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE PANDA_GIT_COMMIT_STR
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# Separate the plugin version into its three components.
string(REPLACE "." ";" P3D_PLUGIN_VERSION_LIST "${P3D_PLUGIN_VERSION}")
list(GET P3D_PLUGIN_VERSION_LIST 0 P3D_PLUGIN_MAJOR_VERSION)
list(GET P3D_PLUGIN_VERSION_LIST 1 P3D_PLUGIN_MINOR_VERSION)
list(GET P3D_PLUGIN_VERSION_LIST 2 P3D_PLUGIN_SEQUENCE_VERSION)

set(P3D_PLUGIN_VERSION_STR "${P3D_PLUGIN_VERSION}${VERSION_SUFFIX}")

# The plugin version as dot-delimited integer quad, according to MS
# conventions for DLL version numbers.
if(PANDA_OFFICIAL_VERSION)
  set(P3D_PLUGIN_DLL_DOT_VERSION "${P3D_PLUGIN_VERSION}.1000")
else()
  set(P3D_PLUGIN_DLL_DOT_VERSION "${P3D_PLUGIN_VERSION}.0")
endif()

# The same thing as a comma-delimited quad.
string(REPLACE "." "," P3D_PLUGIN_DLL_COMMA_VERSION "${P3D_PLUGIN_DLL_DOT_VERSION}")

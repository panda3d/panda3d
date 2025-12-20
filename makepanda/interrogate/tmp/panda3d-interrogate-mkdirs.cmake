# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate")
  file(MAKE_DIRECTORY "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate")
endif()
file(MAKE_DIRECTORY
  "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate-build"
  "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate"
  "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/tmp"
  "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate-stamp"
  "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src"
  "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate-stamp"
)

set(configSubDirs Standard;Release;RelWithDebInfo;Debug;MinSizeRel)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/trici/Documents/GitHub/panda3d/makepanda/interrogate/src/panda3d-interrogate-stamp${cfgdir}") # cfgdir has leading slash
endif()

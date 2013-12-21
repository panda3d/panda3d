# Filename: ConfigPackage.cmake
# Author: kestred (30 Nov, 2013)
#
# This modules defines a function which finds and configures libraries
# and packages for Panda3Ds configuration file headers (.h files).
#
# Assumes the file has already been found with find_package().
#
# Usage:
#   config_package(package_name)
#
# The following variables can be set to override the cached values of USE_XYZZY
#    CONFIG_DISABLE_EVERYTHING - always set USE_XYZZY to false
#    CONFIG_DISABLE_MISSING    - set USE_XYZZY to false if it was not found
#    CONFIG_ENABLE_FOUND       - set USE_XYZZY to true if the package was found
#    CONFIG_ENABLE_EVERYTHING  - always set USE_XYZZY to true (even if the package is missing)
#

# Usage:
#   config_package(PACKAGE_NAME [<display-name>])
function(config_package PKG_NAME)
  # Get package display name
  foreach(arg ${ARGN})
    set(DISPLAY_NAME "${DISPLAY_NAME}${arg}")
  endforeach()

  # Set function vars to defaults
  if(NOT DISPLAY_NAME)
    set(DISPLAY_NAME ${PKG_NAME})
  endif()

  if(${PKG_NAME}_FOUND)
    # Output success after finding the package for the first time
    if(NOT DEFINED USE_${PKG_NAME} AND NOT CONFIG_DISABLE_EVERYTHING)
      message(STATUS "+ ${DISPLAY_NAME}")
    endif()

    ### Add a USE_XYZZY config variable to the cache ###
    if(CONFIG_ENABLE_EVERYTHING OR CONFIG_DISABLE_EVERYTHING OR CONFIG_ENABLE_FOUND)
      unset(USE_${PKG_NAME} CACHE)
    endif()

    if(CONFIG_DISABLE_EVERYTHING)
      option(USE_${PKG_NAME} "If on, compile Panda3D with ${DISPLAY_NAME}" OFF)
    else()
      option(USE_${PKG_NAME} "If on, compile Panda3D with ${DISPLAY_NAME}" ON)
    endif()


    # Update HAVE_XYZZY
    if(USE_${PKG_NAME})
      set(HAVE_${PKG_NAME} TRUE PARENT_SCOPE)
    endif()

  elseif(DEFINED USE_${PKG_NAME})
    # If we were compiling with a particular package, but we can't find it;
    #     then inform the user the package was lost.
    if(USE_${PKG_NAME})
      message(STATUS "- Can no longer find ${DISPLAY_NAME}")

      # Only unset if USE_XYZZY is true;
      # This allows us to set USE_XYZZY to false to silence the output
      unset(USE_${PKG_NAME} CACHE)

      # If using Discovery, we want to silently disable missing packages
      if(CONFIG_DISABLE_MISSING)
        option(USE_${PKG_NAME} "If on, compile Panda3D with ${DISPLAY_NAME}" OFF)
      endif()
    endif()

  else()
    # If using DISCOVERED <OR> NOTHING, we want to silently disable missing packages
    if(CONFIG_DISABLE_MISSING OR CONFIG_DISABLE_EVERYTHING)
      option(USE_${PKG_NAME} "If on, compile Panda3D with ${DISPLAY_NAME}" OFF)

    else()
      # Otherwise, output failure to find package
      message(STATUS "- Did not find ${DISPLAY_NAME}")
    endif()
  endif()
endfunction()

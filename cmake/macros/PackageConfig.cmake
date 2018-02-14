# Filename: PackageConfig.cmake
#
# This module defines functions which find and configure libraries
# and packages for Panda3D.
#
# Assumes an attempt to find the package has already been made with
# find_package(). (i.e. relies on packagename_FOUND variable)
#
# Function: package_option
# Usage:
#   package_option(package_name package_doc_string
#                  [DEFAULT ON | OFF]
#                  [LICENSE license])
# Examples:
#   package_option(LIBNAME "Enables LIBNAME support." DEFAULT OFF)
#
#       If no default is given, the default in normal
#       builds is to enable all found third-party packages.
#       In builds for redistribution, there is the additional requirement that
#       the package be suitably-licensed.
#
#
# Function: config_package
# Usage:
#   config_package(package_name "Package description" ["Config summary"])
# Examples:
#   config_package(OpenAL "OpenAL Audio Output")
#   config_package(ROCKET "Rocket" "without Python bindings")
#
#
# Function: show_packages
# Usage:
#   show_packages()
#
#   This prints the package usage report using the information provided in
#   calls to config_package above.
#
#
# Function: target_use_packages
# Usage:
#   target_use_packages(target [PACKAGES ...])
# Examples:
#   target_use_packages(mylib PYTHON PNG)
#

#
# package_option
#
# In order to make sure no third-party licenses are inadvertently violated,
# this imposes a few rules regarding license:
# 1) If there is no license, no special restrictions.
# 2) If there is a license, but the build is not flagged for redistribution,
#    no special restrictions.
# 3) If there is a license, and this is for redistribution, the package is
#    forcibly defaulted off and must be explicitly enabled, unless the license
#    matches a list of licenses suitable for redistribution.
#
function(package_option name)
  # Parse the arguments.
  set(command)
  set(default)
  set(license "")
  set(cache_string)

  foreach(arg ${ARGN})
    if(command STREQUAL "DEFAULT")
      set(default "${arg}")
      set(command)

    elseif(command STREQUAL "LICENSE")
      set(license "${arg}")
      set(command)

    elseif(arg STREQUAL "DEFAULT")
      set(command "DEFAULT")

    elseif(arg STREQUAL "LICENSE")
      set(command "LICENSE")

    else()
      # Yes, a list, because semicolons can be in there, and
      # that gets split up into multiple args, so we have to
      # join it back together here.
      list(APPEND cache_string "${arg}")

    endif()
  endforeach()

  if(command STREQUAL "DEFAULT")
    message(SEND_ERROR "DEFAULT in package_option takes an argument")    
  endif()

  # If the default is not set, we set it.
  if(NOT DEFINED default)
    if(IS_DIST_BUILD)
      # Accept things that don't have a configured license
      if(license STREQUAL "")
        set(default "${${name}_FOUND}")

      else()
        list(FIND PANDA_DIST_USE_LICENSES ${license} license_index)
        # If the license isn't in the accept listed, don't use the package
        message("INDEX for ${name}: ${license_index}")
        if(${license_index} EQUAL "-1")
          set(default OFF)
        else()
          set(default "${${name}_FOUND}")
        endif()
      endif()

    elseif(IS_MINSIZE_BUILD)
      set(default OFF)

    else()
      set(default "${${name}_FOUND}")

    endif()
  endif()

  # If it was set by the user but not found, display an error.
  if(HAVE_${name} AND NOT ${name}_FOUND)
    message(SEND_ERROR "NOT FOUND: ${name}.  Disable HAVE_${name} to continue.")
  endif()

  # Prevent the function from being called twice.
  #   This would indicate a cmake error.
  if(PANDA_DID_SET_OPTION_${name})
    message(SEND_ERROR "package_option(${name}) was called twice.
                        This is a bug in the cmake build scripts.")
  else()
    set(PANDA_DID_SET_OPTION_${name} TRUE PARENT_SCOPE)
  endif()

  set(PANDA_PACKAGE_DEFAULT_${name} "${default}")

  # Create the option.
  option("HAVE_${name}" "${cache_string}" "${default}")
  if(HAVE_${name})
    set(_${name}_LIBRARY ${${name}_LIBRARY} CACHE INTERNAL "<Internal>")
    set(_${name}_LIBRARIES ${${name}_LIBRARIES} CACHE INTERNAL "<Internal>")
  else()
    unset(_${name}_LIBRARY CACHE)
    unset(_${name}_LIBRARIES CACHE)
  endif()
endfunction()

set(_ALL_CONFIG_PACKAGES CACHE INTERNAL "Internal variable")

#
# config_package
#
function(config_package name desc)
  set(note "")
  foreach(arg ${ARGN})
    set(note "${arg}")
  endforeach()

  if(NOT PANDA_DID_SET_OPTION_${name})
    message(SEND_ERROR "config_package(${name}) was called before package_option(${name}).
                        This is a bug in the cmake build scripts.")
  endif()

  list(FIND _ALL_CONFIG_PACKAGES "${name}" called_twice)
  if(called_twice GREATER -1)
    message(SEND_ERROR "config_package(${name}) was called twice.
                        This is a bug in the cmake build scripts.")
  else()
    list(APPEND _ALL_CONFIG_PACKAGES "${name}")
    set(_ALL_CONFIG_PACKAGES "${_ALL_CONFIG_PACKAGES}" CACHE INTERNAL "Internal variable")
  endif()

  set(PANDA_PACKAGE_DESC_${name} "${desc}" PARENT_SCOPE)
  set(PANDA_PACKAGE_NOTE_${name} "${note}" PARENT_SCOPE)
endfunction()

#
# show_packages
#
function(show_packages)
  message("")
  message("Configuring support for the following optional third-party packages:")

  foreach(package ${_ALL_CONFIG_PACKAGES})
    set(desc "${PANDA_PACKAGE_DESC_${package}}")
    set(note "${PANDA_PACKAGE_NOTE_${package}}")
    if(HAVE_${package})
      if(NOT note STREQUAL "")
        message("+ ${desc} (${note})")
      else()
        message("+ ${desc}")
      endif()
    else()
      if(NOT ${package}_FOUND)
        set(reason "not found")
      elseif(PANDA_PACKAGE_DEFAULT_${package})
        set(reason "not requested")
      else()
        set(reason "disabled")
      endif()
      message("- ${desc} (${reason})")
    endif()
  endforeach()
endfunction()

#
# target_use_packages
#
# Useful macro that picks up a package located using find_package
# as dependencies of a target that is going to be built.
#
macro(target_use_packages target)
  set(libs ${ARGV})
  list(REMOVE_AT libs 0)

  foreach(lib ${libs})
    if(HAVE_${lib})
      # N.B. target_include_directories is new in CMake 2.8.11; we target 2.8.4.
      #target_include_directories("${target}" PUBLIC "${${lib}_INCLUDE_DIRS};${${lib}_INCLUDE_DIR}")
      include_directories("${${lib}_INCLUDE_DIRS};${${lib}_INCLUDE_DIR}")
      if(_${lib}_LIBRARIES)
        target_link_libraries("${target}" ${_${lib}_LIBRARIES})
      else()
        target_link_libraries("${target}" ${_${lib}_LIBRARY})
      endif()
    endif()
  endforeach(lib)
endmacro(target_use_packages)

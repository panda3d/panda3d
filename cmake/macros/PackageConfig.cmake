# Filename: PackageConfig.cmake
#
# This modules defines functions which find and configures libraries
# and packages for Panda3Ds configuration file headers (.h files).
#
# Assumes the file has already been found with find_package().
#
# Function: package_option
# Usage:
#   package_option(package_name DOCSTRING
#                  [DEFAULT ON | OFF]
#                  [LICENSE license])
# Examples:
#   package_option(LIBNAME "Enables LIBNAME support." DEFAULT OFF)
#
#       If no default is given, the default in normal
#       builds is to enable all found third-party packages.
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
      target_include_directories("${target}" PUBLIC "${${lib}_INCLUDE_DIRS};${${lib}_INCLUDE_DIR}")
      if(_${lib}_LIBRARIES)
        target_link_libraries("${target}" ${_${lib}_LIBRARIES})
      else()
        target_link_libraries("${target}" ${_${lib}_LIBRARY})
      endif()
    endif()
  endforeach(lib)
endmacro(target_use_packages)

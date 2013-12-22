# Filename: PackageConfig.cmake
#
# This modules defines functions which find and configures libraries
# and packages for Panda3Ds configuration file headers (.h files).
#
# Assumes the file has already been found with find_package().
#
# Function: package_option
# Usage:
#   package_option(package_name DOCSTRING [DEFAULT ON | OFF])
# Examples:
#   add_library(mylib ${SOURCES})
#
#       If no default is given, the default is whether the package was found.
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
  set(cache_string)

  foreach(arg ${ARGN})
    if(command STREQUAL "DEFAULT")
      set(default "${arg}")
      set(command)

    elseif(arg STREQUAL "DEFAULT")
      set(command "DEFAULT")

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
    set(default "${${name}_FOUND}")
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
      if(${lib}_LIBRARIES)
        target_link_libraries("${target}" ${${lib}_LIBRARIES})
      else()
        target_link_libraries("${target}" ${${lib}_LIBRARY})
      endif()
    endif()
  endforeach(lib)
endmacro(target_use_packages)

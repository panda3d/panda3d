# Filename: PackageConfig.cmake
#
# This module defines functions which find and configure libraries
# and packages for Panda3D.
#
# Assumes an attempt to find the package has already been made with
# find_package(). (i.e. relies on packagename_FOUND variable)
#
# The packages are added as imported/interface libraries in the PKG::
# namespace.  If the package is not found (or disabled by the user),
# a dummy package will be created instead.  Therefore, it is safe
# to link against the PKG::PACKAGENAME target unconditionally.
#
# Function: package_option
# Usage:
#   package_option(package_name package_doc_string
#                  [DEFAULT ON | OFF]
#                  [IMPORTED_AS CMake::Imported::Target [...]]
#                  [FOUND_AS find_name]
#                  [LICENSE license])
# Examples:
#   package_option(LIBNAME "Enables LIBNAME support." DEFAULT OFF)
#
#       If no default is given, the default in normal
#       builds is to enable all found third-party packages.
#       In builds for redistribution, there is the additional requirement that
#       the package be suitably-licensed.
#
#       FOUND_AS indicates the name of the CMake find_package() module, which
#       may differ from Panda3D's internal name for that package.
#
#       IMPORTED_AS is used to indicate that the find_package() may have
#       provided one or more IMPORTED targets, and that if at least one is
#       found, the IMPORTED target(s) should be used instead of the
#       variables provided by find_package()
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
  set(found_as "${name}")
  set(imported_as)
  set(license "")
  set(cache_string)

  foreach(arg ${ARGN})
    if(command STREQUAL "DEFAULT")
      set(default "${arg}")
      set(command)

    elseif(command STREQUAL "FOUND_AS")
      set(found_as "${arg}")
      set(command)

    elseif(command STREQUAL "LICENSE")
      set(license "${arg}")
      set(command)

    elseif(arg STREQUAL "DEFAULT")
      set(command "DEFAULT")

    elseif(arg STREQUAL "FOUND_AS")
      set(command "FOUND_AS")

    elseif(arg STREQUAL "LICENSE")
      set(command "LICENSE")

    elseif(arg STREQUAL "IMPORTED_AS")
      set(command "IMPORTED_AS")

    elseif(command STREQUAL "IMPORTED_AS")
      list(APPEND imported_as "${arg}")

    else()
      # Yes, a list, because semicolons can be in there, and
      # that gets split up into multiple args, so we have to
      # join it back together here.
      list(APPEND cache_string "${arg}")

    endif()
  endforeach()

  if(command AND NOT command STREQUAL "IMPORTED_AS")
    message(SEND_ERROR "${command} in package_option takes an argument")
  endif()

  # If the default is not set, we set it.
  if(NOT DEFINED default)
    if(IS_DIST_BUILD)
      # Accept things that don't have a configured license
      if(license STREQUAL "")
        set(default "${${found_as}_FOUND}")

      else()
        list(FIND PANDA_DIST_USE_LICENSES ${license} license_index)
        # If the license isn't in the accept listed, don't use the package
        if(${license_index} EQUAL "-1")
          set(default OFF)
        else()
          set(default "${${found_as}_FOUND}")
        endif()
      endif()

    elseif(IS_MINSIZE_BUILD)
      set(default OFF)

    else()
      set(default "${${found_as}_FOUND}")

    endif()
  endif()

  # If it was set by the user but not found, display an error.
  if(HAVE_${name} AND NOT ${found_as}_FOUND)
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

  set(PANDA_PACKAGE_DEFAULT_${name} "${default}" PARENT_SCOPE)

  # Create the INTERFACE library used to depend on this package.
  add_library(PKG::${name} INTERFACE IMPORTED GLOBAL)

  # Create the option, and if it actually is enabled, populate the INTERFACE
  # library created above
  option("HAVE_${name}" "${cache_string}" "${default}")
  if(HAVE_${name})
    set(use_variables ON)

    foreach(implib ${imported_as})
      if(TARGET ${implib})
        # We found one of the implibs, so we don't need to use variables
        # (below) anymore
        set(use_variables OFF)

        # Yes, this is ugly.  See below for an explanation.
        target_link_libraries(PKG::${name} INTERFACE
          "$<$<NOT:$<BOOL:$<TARGET_PROPERTY:IS_INTERROGATE>>>:${implib}>")
      endif()
    endforeach(implib)

    if(use_variables)
      if(${found_as}_INCLUDE_DIRS)
        set(includes ${${found_as}_INCLUDE_DIRS})
      else()
        set(includes "${${found_as}_INCLUDE_DIR}")
      endif()
      if(${found_as}_LIBRARIES)
        set(libs ${${found_as}_LIBRARIES})
      else()
        set(libs "${${found_as}_LIBRARY}")
      endif()

      target_link_libraries(PKG::${name} INTERFACE ${libs})

      # This is gross, but we actually want to hide package include directories
      # from Interrogate to make sure it relies on parser-inc instead, so we'll
      # use some generator expressions to do that.
      set_target_properties(PKG::${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
        "$<$<NOT:$<BOOL:$<TARGET_PROPERTY:IS_INTERROGATE>>>:${includes}>")
    endif()
  endif()
endfunction(package_option)

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
      elseif(NOT PANDA_PACKAGE_DEFAULT_${package})
        set(reason "not requested")
      else()
        set(reason "disabled")
      endif()
      message("- ${desc} (${reason})")
    endif()
  endforeach()
endfunction()

#
# find_package
#
# This override is necessary because CMake's default behavior is to run
# find_package in MODULE mode, *then* in CONFIG mode.  This is silly!  CONFIG
# mode makes more sense to be done first, since any system config file will
# know vastly more about the package's configuration than a module can hope to
# guess.
#
macro(find_package name)
  if(";${ARGN};" MATCHES ";(CONFIG|MODULE|NO_MODULE);")
    # Caller explicitly asking for a certain mode; so be it.
    _find_package(${ARGV})
  else()
    string(TOUPPER "${name}" __pkgname_upper)

    # Try CONFIG
    _find_package("${name}" CONFIG ${ARGN})
    if(NOT ${name}_FOUND)
      # CONFIG didn't work, fall back to MODULE
      _find_package("${name}" MODULE ${ARGN})
    else()
      # Case-sensitivity
      set(${__pkgname_upper}_FOUND 1)
    endif()
  endif()
endmacro(find_package)

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
#
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
# Function: package_status
# Usage:
#   package_status(package_name "Package description" ["Config summary"])
#
# Examples:
#   package_status(OpenAL "OpenAL Audio Output")
#   package_status(ROCKET "Rocket" "without Python bindings")
#
#
# Function: show_packages
# Usage:
#   show_packages()
#
#   This prints the package usage report using the information provided in
#   calls to package_status above.
#

set(_ALL_PACKAGE_OPTIONS CACHE INTERNAL "Internal variable")

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

  string(TOUPPER "${name}" name)

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

    else()
      set(default "${${found_as}_FOUND}")

    endif()
  endif()

  option("HAVE_${name}" "${cache_string}" "${default}")

  # If it was set by the user but not found, display an error.
  string(TOUPPER "${found_as}" FOUND_AS)
  if(HAVE_${name} AND NOT ${found_as}_FOUND AND NOT ${FOUND_AS}_FOUND)
    message(SEND_ERROR "NOT FOUND: ${name}.  Disable HAVE_${name} to continue.")
  endif()

  # Prevent the function from being called twice.
  #   This would indicate a cmake error.
  if(";${_ALL_PACKAGE_OPTIONS};" MATCHES ";${name};")
    message(SEND_ERROR "package_option(${name}) was called twice.
                        This is a bug in the cmake build scripts.")

  else()
    list(APPEND _ALL_PACKAGE_OPTIONS "${name}")
    set(_ALL_PACKAGE_OPTIONS "${_ALL_PACKAGE_OPTIONS}" CACHE INTERNAL "Internal variable")

  endif()

  set(PANDA_PACKAGE_DEFAULT_${name} "${default}" PARENT_SCOPE)

  if(${found_as}_FOUND OR ${FOUND_AS}_FOUND)
    set(PANDA_PACKAGE_FOUND_${name} ON PARENT_SCOPE)
  else()
    set(PANDA_PACKAGE_FOUND_${name} OFF PARENT_SCOPE)
  endif()

  # Create the INTERFACE library used to depend on this package.
  add_library(PKG::${name} INTERFACE IMPORTED GLOBAL)

  # If the option actually is enabled, populate the INTERFACE library created above
  if(HAVE_${name})
    set(use_variables ON)

    # This is gross, but we actually want to hide package include directories
    # from Interrogate to make sure it relies on parser-inc instead, so we'll
    # use some generator expressions to do that.
    set(_is_not_interface_lib
      "$<NOT:$<STREQUAL:$<TARGET_PROPERTY:TYPE>,INTERFACE_LIBRARY>>")
    set(_is_not_interrogate
      "$<NOT:$<BOOL:$<${_is_not_interface_lib}:$<TARGET_PROPERTY:IS_INTERROGATE>>>>")

    foreach(implib ${imported_as})
      if(TARGET ${implib})
        # We found one of the implibs, so we don't need to use variables
        # (below) anymore
        set(use_variables OFF)

        # Hide it from Interrogate
        target_link_libraries(PKG::${name} INTERFACE
          "$<${_is_not_interrogate}:$<TARGET_NAME:${implib}>>")
      endif()
    endforeach(implib)

    if(use_variables)
      if(DEFINED ${found_as}_INCLUDE_DIRS)
        set(includes ${${found_as}_INCLUDE_DIRS})
      elseif(DEFINED ${found_as}_INCLUDE_DIR)
        set(includes "${${found_as}_INCLUDE_DIR}")
      elseif(DEFINED ${FOUND_AS}_INCLUDE_DIRS)
        set(includes ${${FOUND_AS}_INCLUDE_DIRS})
      else()
        set(includes "${${FOUND_AS}_INCLUDE_DIR}")
      endif()

      if(DEFINED ${found_as}_LIBRARIES)
        set(libs ${${found_as}_LIBRARIES})
      elseif(DEFINED ${found_as}_LIBRARY)
        set(libs "${${found_as}_LIBRARY}")
      elseif(DEFINED ${FOUND_AS}_LIBRARIES)
        set(libs ${${FOUND_AS}_LIBRARIES})
      else()
        set(libs "${${FOUND_AS}_LIBRARY}")
      endif()

      target_link_libraries(PKG::${name} INTERFACE ${libs})

      # Hide it from Interrogate
      set_target_properties(PKG::${name} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "$<${_is_not_interrogate}:${includes}>")
    endif()
  endif()
endfunction(package_option)

set(_ALL_CONFIG_PACKAGES CACHE INTERNAL "Internal variable")

#
# package_status
#
function(package_status name desc)
  set(note "")
  foreach(arg ${ARGN})
    set(note "${arg}")
  endforeach()

  string(TOUPPER "${name}" name)

  if(NOT ";${_ALL_PACKAGE_OPTIONS};" MATCHES ";${name};")
    message(SEND_ERROR "package_status(${name}) was called before package_option(${name}).
                        This is a bug in the cmake build scripts.")
    return()
  endif()

  if(";${_ALL_CONFIG_PACKAGES};" MATCHES ";${name};")
    message(SEND_ERROR "package_status(${name}) was called twice.
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

    if(HAVE_${package} AND PANDA_PACKAGE_FOUND_${package})
      if(NOT note STREQUAL "")
        message("+ ${desc} (${note})")
      else()
        message("+ ${desc}")
      endif()

    elseif(HAVE_${package})
      message("! ${desc} (enabled but not found)")

    else()
      if(NOT PANDA_PACKAGE_FOUND_${package})
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
# export_packages(filename)
#
# Generates an includable CMake file that contains definitions for every PKG::
# package defined.
#
function(export_packages filename)
  set(exports "# Exports for Panda3D PKG:: packages\n")

  foreach(pkg ${_ALL_PACKAGE_OPTIONS})
    set(exports "${exports}\n# Create imported target PKG::${pkg}\n")
    set(exports "${exports}add_library(PKG::${pkg} INTERFACE IMPORTED)\n\n")

    set(exports "${exports}set_target_properties(PKG::${pkg} PROPERTIES\n")
    foreach(prop
        INTERFACE_COMPILE_DEFINITIONS
        INTERFACE_COMPILE_FEATURES
        INTERFACE_COMPILE_OPTIONS
        INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_LINK_DEPENDS
        INTERFACE_LINK_DIRECTORIES
        INTERFACE_LINK_OPTIONS
        INTERFACE_POSITION_INDEPENDENT_CODE
       #INTERFACE_SYSTEM_INCLUDE_DIRECTORIES  # Let the consumer dictate this
        INTERFACE_SOURCES)

      set(prop_ex "$<TARGET_PROPERTY:PKG::${pkg},${prop}>")
      set(exports "${exports}$<$<BOOL:${prop_ex}>:  ${prop} \"${prop_ex}\"\n>")

    endforeach(prop)

    # Ugh, INTERFACE_LINK_LIBRARIES isn't transitive.  Fine.  Take care of it
    # by hand:
    set(libraries)
    set(stack "PKG::${pkg}")
    set(history)
    while(stack)
      # Remove head item from stack
      unset(head)
      while(NOT DEFINED head)
        if(NOT stack)
          break()
        endif()

        list(GET stack 0 head)
        list(REMOVE_AT stack 0)

        # Don't visit anything twice
        list(FIND history "${head}" _index)
        if(_index GREATER -1)
          unset(head)
        endif()
      endwhile()

      if(head)
        list(APPEND history "${head}")
      else()
        break()
      endif()

      # If head isn't a target, add it to `libraries`, else recurse
      if(TARGET "${head}")
        get_target_property(link_libs "${head}" INTERFACE_LINK_LIBRARIES)
        if(link_libs)
          list(APPEND stack ${link_libs})
        endif()

        get_target_property(type "${head}" TYPE)
        if(NOT type STREQUAL "INTERFACE_LIBRARY")
          get_target_property(imported_location "${head}" IMPORTED_LOCATION)
          get_target_property(imported_implib "${head}" IMPORTED_IMPLIB)
          if(imported_implib)
            list(APPEND libraries ${imported_implib})
          elseif(imported_location)
            list(APPEND libraries ${imported_location})
          endif()

          get_target_property(configs "${head}" IMPORTED_CONFIGURATIONS)
          if(configs AND NOT imported_location)
            foreach(config ${configs})
              get_target_property(imported_location "${head}" IMPORTED_LOCATION_${config})

              # Prefer IMPORTED_IMPLIB where present
              get_target_property(imported_implib "${head}" IMPORTED_IMPLIB_${config})
              if(imported_implib)
                set(imported_location "${imported_implib}")
              endif()

              if(imported_location)
                if(configs MATCHES ".*;.*")
                  set(_bling "$<1:$>") # genex-escaped $
                  list(APPEND libraries "${_bling}<${_bling}<CONFIG:${config}>:${imported_location}>")

                else()
                  list(APPEND libraries ${imported_location})

                endif()
              endif()
            endforeach(config)
          endif()

        else()
          # This is an INTERFACE_LIBRARY.
          get_target_property(imported_libname "${head}" IMPORTED_LIBNAME)
          if(imported_libname)
            list(APPEND libraries ${imported_libname})
          endif()

        endif()

      elseif("${head}" MATCHES "\\$<TARGET_NAME:\([^>]+\)>")
        string(REGEX REPLACE ".*\\$<TARGET_NAME:\([^>]+\)>.*" "\\1" match "${head}")
        list(APPEND stack "${match}")

      else()
        list(APPEND libraries "${head}")

      endif()
    endwhile(stack)

    set(exports "${exports}  INTERFACE_LINK_LIBRARIES \"${libraries}\"\n")

    set(exports "${exports})\n")
  endforeach(pkg)

  # file(GENERATE) does not like $<LINK_ONLY:...> (and it's meant to be
  # consumed by our importer) so we escape it
  set(_bling "$<1:$>") # genex-escaped $
  string(REPLACE "$<LINK_ONLY:" "${_bling}<LINK_ONLY:" exports "${exports}")

  file(GENERATE OUTPUT "${filename}" CONTENT "${exports}")
endfunction(export_packages)

#
# export_targets(set [NAMESPACE namespace] [COMPONENT component])
#
# Export targets in the export set named by "set"
#
# NAMESPACE overrides the namespace prefixed to the exported targets; it
# defaults to "Panda3D::[set]::" if no explicit override is given.
#
# COMPONENT overrides the install component for the generated .cmake file; it
# defaults to "[set]" if no explicit override is given.
#
function(export_targets set)
  set(namespace "Panda3D::${set}::")
  set(component "${set}")
  set(keyword)
  foreach(arg ${ARGN})
    if(arg STREQUAL "NAMESPACE" OR
       arg STREQUAL "COMPONENT")

      set(keyword "${arg}")

    elseif(keyword STREQUAL "NAMESPACE")
      set(namespace "${arg}")

    elseif(keyword STREQUAL "COMPONENT")
      set(component "${arg}")

    else()
      message(FATAL_ERROR "export_targets() given unexpected arg: ${arg}")

    endif()
  endforeach(arg)

  export(EXPORT "${set}" NAMESPACE "${namespace}"
    FILE "${PROJECT_BINARY_DIR}/Panda3D${set}Targets.cmake")
  install(EXPORT "${set}" NAMESPACE "${namespace}"
    FILE "Panda3D${set}Targets.cmake"
    COMPONENT "${component}" DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Panda3D)

endfunction(export_targets)

#
# find_package
#
# This override implements CMAKE_FIND_PACKAGE_PREFER_CONFIG on versions of
# CMake too old to include it.
#
if(CMAKE_VERSION VERSION_LESS "3.15")
  macro(find_package name)
    if(";${ARGN};" MATCHES ";(CONFIG|MODULE|NO_MODULE);")
      # Caller explicitly asking for a certain mode; so be it.
      _find_package(${ARGV})

    elseif(CMAKE_FIND_PACKAGE_PREFER_CONFIG)
      # Try CONFIG
      _find_package("${name}" CONFIG ${ARGN})

      if(NOT ${name}_FOUND)
        # CONFIG didn't work, fall back to MODULE
        _find_package("${name}" MODULE ${ARGN})
      endif()

    else()
      # Default behavior
      _find_package(${ARGV})

    endif()
  endmacro(find_package)
endif()

# Filename: PerConfigOption.cmake
#
# This contains a convenience function for defining per-config options.
# In single-config generators, it will set the option based on the defined
# CMAKE_BUILD_TYPE.  In multi-config generators, it will create separate
# options, one per config.
#
# Function: per_config_option
# Usage:
#   option(name "help string" [Config1] [Config2] [...ConfigN])
#
# Example:
#   per_config_option(DO_DEBUGGING "Enables debugging." Debug Standard)

set(_PER_CONFIG_OPTIONS CACHE INTERNAL "Internal variable")

function(per_config_option name help)
  set(_configs ${ARGN})

  # In single-config generatotrs, we simply create one config.
  if(NOT IS_MULTICONFIG)
    list(FIND _configs "${CMAKE_BUILD_TYPE}" _index)
    if(${_index} GREATER -1)
      option("${name}" "${help}" ON)
    else()
      option("${name}" "${help}" OFF)
    endif()

  elseif(DEFINED "${name}")
    # It's been explicitly defined, so that makes it not a multi-configuration
    # variable anymore.
    option("${name}" "${help}")
    return()

  else()
    foreach(_config ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${_config}" _config_upper)
      list(FIND _configs "${_config}" _index)
      if(${_index} GREATER -1)
        option("${name}_${_config_upper}" "${help}" ON)
      else()
        option("${name}_${_config_upper}" "${help}" OFF)
      endif()
    endforeach()

  endif()

  list(APPEND _PER_CONFIG_OPTIONS "${name}")
  set(_PER_CONFIG_OPTIONS "${_PER_CONFIG_OPTIONS}" CACHE INTERNAL "Internal variable")
endfunction(per_config_option)

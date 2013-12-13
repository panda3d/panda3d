## Parse Panda3D Subversions ##
list(GET PANDA_VERSION 0 PANDA_MAJOR_VERSION)
list(GET PANDA_VERSION 1 PANDA_MINOR_VERSION)
list(GET PANDA_VERSION 2 PANDA_SEQUENCE_VERSION)

string(REPLACE ";" "." PANDA_VERSION_STR "${PANDA_VERSION}")
if(NOT PANDA_OFFICIAL_VERSION)
	set(PANDA_VERSION_STR "${PANDA_VERSION_STR}c")
endif()

set(PANDA_VERSION_SYMBOL panda_version_${PANDA_MAJOR_VERSION}_${PANDA_MINOR_VERSION})

math(EXPR PANDA_NUMERIC_VERSION "${PANDA_MAJOR_VERSION}*1000000 + ${PANDA_MINOR_VERSION}*1000 + ${PANDA_SEQUENCE_VERSION}")


## Parse Plugin-framework Subversions ##
list(GET P3D_PLUGIN_VERSION 0 P3D_PLUGIN_MAJOR_VERSION)
list(GET P3D_PLUGIN_VERSION 1 P3D_PLUGIN_MINOR_VERSION)
list(GET P3D_PLUGIN_VERSION 2 P3D_PLUGIN_SEQUENCE_VERSION)

string(REPLACE ";" "." P3D_PLUGIN_VERSION_STR "${P3D_PLUGIN_VERSION}")
if(PANDA_OFFICIAL_VERSION)
	set(P3D_PLUGIN_DLL_DOT_VERSION "${P3D_PLUGIN_VERSION_STR}.1000")
else()
	set(P3D_PLUGIN_DLL_DOT_VERSION "${P3D_PLUGIN_VERSION_STR}.0")
	set(P3D_PLUGIN_VERSION_STR "${P3D_PLUGIN_VERSION_STR}c")
endif()
string(REPLACE "." "," P3D_PLUGIN_DLL_COMMA_VERSION "${P3D_PLUGIN_DLL_DOT_VERSION}")

# PRC Config settings
set(PRC_DEFAULT_DIR_ABSOLUTE "" CACHE STRING "Specify the PRC_DEFAULT_DIR as an absolute path.")
if(NOT PRC_DEFAULT_DIR_ABSOLUTE)
	set(PRC_DEFAULT_DIR "etc" CACHE STRING
	"Panda uses prc files for runtime configuration. Panda will search the default
	.prc directory if the PRC_PATH and PRC_DIR environment variables are not defined.")
else()
	set(PRC_DEFAULT_DIR "" CACHE STRING
	"Panda uses prc files for runtime configuration. Panda will search the default
	.prc directory if the PRC_PATH and PRC_DIR environment variables are not defined.")
endif()
set(PRC_DCONFIG_TRUST_LEVEL 0 CACHE STRING "The trust level value for any legacy (DConfig) variables.")
set(PRC_INC_TRUST_LEVEL 0 CACHE STRING "The amount by which we globally increment the trust level.")
if(PRC_DEFAULT_DIR AND PRC_DEFAULT_DIR_ABSOLUTE)
	message(FATAL_ERROR "Cannot have both PRC_DEFAULT_DIR and PRC_DEFAULT_DIR_ABSOLUTE, choose one.")
endif()

# PRC special values for config headers
if(PRC_DEFAULT_DIR_ABSOLUTE)
	set(DEFAULT_PRC_DIR ${PRC_ABSOLUTE_DIR})
else()
	set(DEFAULT_PRC_DIR "${CMAKE_INSTALL_PREFIX}/panda/${PRC_DEFAULT_DIR}")
endif()



include(Configure.cmake)

## Include dtool subpackages ##
add_subdirectory(src/dtoolbase)
add_subdirectory(src/dtoolutil)
add_subdirectory(src/prc)
add_subdirectory(src/dconfig)
add_subdirectory(src/pystub)
add_subdirectory(src/cppparser)
add_subdirectory(src/interrogatedb)
add_subdirectory(src/interrogate)

## Include dtool metalibs ##
add_subdirectory(metalibs/dtool)

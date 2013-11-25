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


## Installation Config ##
set(INSTALL_DIR "/usr/local/panda" CACHE STRING "The directory in which to install Panda3D.")

set(DTOOL_INSTALL "${INSTALL_DIR}" CACHE STRING "The directory in which to install the dtool package.")

set(DEFAULT_PRC_DIR "${INSTALL_DIR}/etc" CACHE STRING "Panda uses prc files for runtime configuration. Panda will search the default .prc directory if the PRC_PATH and PRC_DIR environment variables are not defined.")


## Include dtool subpackages ##
include(${CMAKE_CURRENT_LIST_DIR}/Configure.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/src/dtoolbase/Sources.cmake)

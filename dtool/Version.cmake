## Panda Version Information ##
set(PANDA_VERSION 1 9 0)
set(PANDA_OFFICIAL_VERSION FALSE CACHE BOOL "If true, signifies this is a tagged release rather than a repository snapshot. Scripts that generate source tarballs and/or binary releases for distribution should explicitly set this to true.")
set(PANDA_DISTRIBUTOR "homebuilt" CACHE STRING "This should be set by whoever provides a particular distribution of Panda. If you build your own Panda, leave this unchanged.")

set(P3D_PLUGIN_VERSION 1 0 4)
set(P3D_COREAPI_VERSION ${P3D_PLUGIN_VERSION} 1)


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
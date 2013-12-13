## Panda Version Information ##
set(PANDA_VERSION "1.9.0" CACHE STRING
  "Use dots to separate the major, minor, and sequence numbers.")
set(PANDA_OFFICIAL_VERSION FALSE CACHE BOOL
  "If on, signifies this is a tagged release rather than a repository snapshot.
Scripts that generate source tarballs and/or binary releases for
distribution should explicitly set this to true.")
set(PANDA_DISTRIBUTOR "homebuilt" CACHE STRING
  "This string is reported verbatim by PandaSystem::get_distributor().
It should be set by whoever provides a particular distribution of Panda.
If you build your own Panda, leave this unchanged.")

set(PANDA_PLUGIN_VERSION "1.1.0" CACHE STRING
  "We also define a version for the Panda3D plugin/runtime. This is an
independent version number from PANDA_VERSION or PANDA_PACKAGE_VERSION,
because it is anticipated that this plugin code, once settled,
will need to be updated much less frequently than Panda itself.")
set(P3D_PLUGIN_VERSION "${PANDA_PLUGIN_VERSION}")
set(P3D_COREAPI_VERSION "${P3D_PLUGIN_VERSION}.1")


## Parse Panda3D Subversions ##
string(REPLACE "." ";" PANDA_VERSION_LIST "${PANDA_VERSION}")
list(GET PANDA_VERSION_LIST 0 PANDA_MAJOR_VERSION)
list(GET PANDA_VERSION_LIST 1 PANDA_MINOR_VERSION)
list(GET PANDA_VERSION_LIST 2 PANDA_SEQUENCE_VERSION)

if(PANDA_OFFICIAL_VERSION)
	set(PANDA_VERSION_STR "${PANDA_VERSION}c")
else()
	set(PANDA_VERSION_STR "${PANDA_VERSION}c")
endif()

set(PANDA_VERSION_SYMBOL panda_version_${PANDA_MAJOR_VERSION}_${PANDA_MINOR_VERSION})

math(EXPR PANDA_NUMERIC_VERSION "${PANDA_MAJOR_VERSION}*1000000 + ${PANDA_MINOR_VERSION}*1000 + ${PANDA_SEQUENCE_VERSION}")


## Parse Plugin-framework Subversions ##
string(REPLACE "." ";" P3D_PLUGIN_VERSION_LIST "${P3D_PLUGIN_VERSION}")
list(GET P3D_PLUGIN_VERSION_LIST 0 P3D_PLUGIN_MAJOR_VERSION)
list(GET P3D_PLUGIN_VERSION_LIST 1 P3D_PLUGIN_MINOR_VERSION)
list(GET P3D_PLUGIN_VERSION_LIST 2 P3D_PLUGIN_SEQUENCE_VERSION)

if(PANDA_OFFICIAL_VERSION)
	set(P3D_PLUGIN_VERSION_STR "${P3D_PLUGIN_VERSION}")
	set(P3D_PLUGIN_DLL_DOT_VERSION "${P3D_PLUGIN_VERSION_STR}.1000")
else()
	set(P3D_PLUGIN_VERSION_STR "${P3D_PLUGIN_VERSION}c")
	set(P3D_PLUGIN_DLL_DOT_VERSION "${P3D_PLUGIN_VERSION}.0")
endif()
string(REPLACE "." "," P3D_PLUGIN_DLL_COMMA_VERSION "${P3D_PLUGIN_DLL_DOT_VERSION}")

include_directories(../dtoolutil)
include_directories(../dtoolbase)
include_directories(../prc)

set(P3DCONFIG_HEADERS
	config_dconfig.h dconfig.I dconfig.h)

set(P3DCONFIG_SOURCES
	config_dconfig.cxx dconfig.cxx)

add_library(p3dconfig STATIC
	${P3DCONFIG_HEADERS}
	${P3DCONFIG_SOURCES})

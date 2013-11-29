include_directories(${CMAKE_CURRENT_LIST_DIR}/../dtoolutil)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../dtoolbase)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../prc)

set(P3DCONFIG_HEADER_FILES
	config_dconfig.h dconfig.I dconfig.h)

set(P3DCONFIG_SOURCE_FILES
	config_dconfig.cxx dconfig.cxx)

foreach(FILENAME IN LISTS P3DCONFIG_HEADER_FILES)
	set(P3DCONFIG_HEADERS ${P3DCONFIG_HEADERS} ${CMAKE_CURRENT_LIST_DIR}/${FILENAME})
endforeach()
foreach(FILENAME IN LISTS P3DCONFIG_SOURCE_FILES)
	set(P3DCONFIG_SOURCES ${P3DCONFIG_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/${FILENAME})
endforeach()

add_library(p3dconfig STATIC
	${P3DCONFIG_HEADERS}
	${P3DCONFIG_SOURCES})

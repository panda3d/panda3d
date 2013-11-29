include_directories(../dtoolbase)

set(P3PYSTUB_HEADERS
	pystub.h)

set(P3PYSTUB_SOURCES
	pystub.cxx)

add_library(p3pystub STATIC
	${P3PYSTUB_HEADERS}
	${P3PYSTUB_SOURCES})

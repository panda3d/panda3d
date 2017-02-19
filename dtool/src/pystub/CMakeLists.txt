set(P3PYSTUB_HEADERS pystub.h)
set(P3PYSTUB_SOURCES pystub.cxx)

add_library(p3pystub STATIC ${P3PYSTUB_HEADERS} ${P3PYSTUB_SOURCES})
target_link_libraries(p3pystub p3dtoolbase)
install(FILES ${P3PYSTUB_HEADERS} DESTINATION include/panda3d)

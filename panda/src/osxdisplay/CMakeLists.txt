if(NOT APPLE OR NOT HAVE_GL OR NOT HAVE_CARBON)
  return()
endif()

set(P3OSXDISPLAY_HEADERS
    config_osxdisplay.h
    osxGraphicsPipe.h
    osxGraphicsWindow.h osxGraphicsWindow.I
    osxGraphicsStateGuardian.h)

set(P3OSXDISPLAY_SOURCES
    config_osxdisplay.cxx osxGraphicsPipe.cxx osxGraphicsStateGuardian.cxx
    osxGraphicsBuffer.cxx)

composite_sources(p3osxdisplay P3OSXDISPLAY_SOURCES)
add_library(p3osxdisplay ${P3OSXDISPLAY_HEADERS} ${P3OSXDISPLAY_SOURCES})
set_target_properties(p3osxdisplay PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_OSXDISPLAY)
target_link_libraries(p3osxdisplay p3display p3putil p3glgsg)

# Frameworks:
find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices)
find_library(CARBON_LIBRARY Carbon)
find_library(AGL_LIBRARY AGL)
find_library(CORESERVICES_LIBRARY CoreServices)
find_library(COCOA_LIBRARY Cocoa)
target_link_libraries(p3osxdisplay
  ${APPLICATIONSERVICES_LIBRARY} ${CARBON_LIBRARY} ${AGL_LIBRARY}
  ${CORESERVICES_LIBRARY} ${COCOA_LIBRARY})

install(TARGETS p3osxdisplay DESTINATION lib)

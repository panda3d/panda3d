if(NOT APPLE OR NOT HAVE_COCOA)
  return()
endif()

set(P3COCOADISPLAY_HEADERS
  config_cocoadisplay.h
  cocoaGraphicsPipe.h cocoaGraphicsPipe.I
  cocoaGraphicsWindow.h cocoaGraphicsWindow.I
  cocoaPandaApp.h
  cocoaPandaView.h
  cocoaPandaWindow.h
  cocoaPandaWindowDelegate.h
  cocoaPandaAppDelegate.h
)

set(P3COCOADISPLAY_SOURCES
  config_cocoadisplay.mm
  cocoaGraphicsPipe.mm
  cocoaGraphicsWindow.mm
  cocoaPandaApp.mm
  cocoaPandaView.mm
  cocoaPandaWindow.mm
  cocoaPandaWindowDelegate.mm
  cocoaPandaAppDelegate.mm
)

composite_sources(p3cocoadisplay P3COCOADISPLAY_SOURCES)
add_component_library(p3cocoadisplay SYMBOL BUILDING_PANDA_COCOADISPLAY
  ${P3COCOADISPLAY_HEADERS} ${P3COCOADISPLAY_SOURCES})
target_link_libraries(p3cocoadisplay panda)

# Frameworks:
find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices)
find_library(APPKIT_LIBRARY AppKit)
find_library(CARBON_LIBRARY Carbon)
find_library(CORE_VIDEO_LIBRARY CoreVideo)
target_link_libraries(p3cocoadisplay
  ${APPLICATIONSERVICES_LIBRARY} ${APPKIT_LIBRARY} ${CARBON_LIBRARY}
  ${CORE_VIDEO_LIBRARY})

mark_as_advanced(
  APPLICATIONSERVICES_LIBRARY APPKIT_LIBRARY CARBON_LIBRARY CORE_VIDEO_LIBRARY)

if(NOT BUILD_METALIBS)
  install(TARGETS p3cocoadisplay EXPORT Core COMPONENT Core DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

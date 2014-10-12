if(APPLE AND HAVE_GL AND HAVE_COCOA)
  set(P3COCOADISPLAY_HEADERS
      config_cocoadisplay.h
      cocoaGraphicsPipe.h cocoaGraphicsPipe.I
      cocoaGraphicsWindow.h cocoaGraphicsWindow.I
      cocoaGraphicsStateGuardian.h cocoaGraphicsStateGuardian.I
      cocoaPandaApp.h cocoaPandaView.h cocoaPandaWindowDelegate.h)

  set(P3COCOADISPLAY_SOURCES
      config_cocoadisplay.mm
      cocoaGraphicsPipe.mm
      cocoaGraphicsStateGuardian.mm
      cocoaGraphicsWindow.mm
      cocoaPandaApp.mm
      cocoaPandaView.mm
      cocoaPandaWindow.mm
      cocoaPandaWindowDelegate.mm)

  #composite_sources(p3cocoadisplay P3COCOADISPLAY_SOURCES)
  add_library(p3cocoadisplay ${P3COCOADISPLAY_HEADERS} ${P3COCOADISPLAY_SOURCES})
  target_link_libraries(p3cocoadisplay p3display p3putil p3glgsg)

  # Frameworks:
  find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices)
  find_library(APPKIT_LIBRARY AppKit)
  find_library(CARBON_LIBRARY Carbon)
  target_link_libraries(p3cocoadisplay
    ${APPLICATIONSERVICES_LIBRARY} ${APPKIT_LIBRARY} ${CARBON_LIBRARY})

  install(TARGETS p3cocoadisplay DESTINATION lib)
endif()

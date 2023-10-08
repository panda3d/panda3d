set(P3TINYDISPLAY_HEADERS
  config_tinydisplay.h
  tinyGeomMunger.I tinyGeomMunger.h
  tinySDLGraphicsPipe.I tinySDLGraphicsPipe.h
  tinySDLGraphicsWindow.I tinySDLGraphicsWindow.h
  tinyGraphicsBuffer.I tinyGraphicsBuffer.h
  tinyGraphicsStateGuardian.I tinyGraphicsStateGuardian.h
  tinyTextureContext.I tinyTextureContext.h
  tinyWinGraphicsPipe.I tinyWinGraphicsPipe.h
  tinyWinGraphicsWindow.I tinyWinGraphicsWindow.h
  tinyXGraphicsPipe.I tinyXGraphicsPipe.h
  tinyXGraphicsWindow.I tinyXGraphicsWindow.h
  tinyOffscreenGraphicsPipe.I tinyOffscreenGraphicsPipe.h
  srgb_tables.h
  zbuffer.h
  zfeatures.h
  zgl.h
  zline.h
  zmath.h
  ztriangle.h
  ztriangle_two.h
  ztriangle_code_1.h
  ztriangle_code_2.h
  ztriangle_code_3.h
  ztriangle_code_4.h
  ztriangle_table.h
  store_pixel.h
  store_pixel_code.h
  store_pixel_table.h
)

set(P3TINYDISPLAY_SOURCES
  clip.cxx
  config_tinydisplay.cxx
  error.cxx
  image_util.cxx
  init.cxx
  td_light.cxx
  memory.cxx
  specbuf.cxx
  store_pixel.cxx
  td_texture.cxx
  tinyGeomMunger.cxx
  tinyGraphicsBuffer.cxx
  tinyGraphicsStateGuardian.cxx
  tinyOffscreenGraphicsPipe.cxx
  tinySDLGraphicsPipe.cxx
  tinySDLGraphicsWindow.cxx
  tinyTextureContext.cxx
  tinyWinGraphicsPipe.cxx
  tinyWinGraphicsWindow.cxx
  tinyXGraphicsPipe.cxx
  tinyXGraphicsWindow.cxx
  vertex.cxx
  ztriangle_table.cxx
  ztriangle_1.cxx
  ztriangle_2.cxx
  ztriangle_3.cxx
  ztriangle_4.cxx
  srgb_tables.cxx
  zbuffer.cxx
  zdither.cxx
  zline.cxx
  zmath.cxx
)

composite_sources(p3tinydisplay P3TINYDISPLAY_SOURCES)
add_component_library(p3tinydisplay SYMBOL BUILDING_TINYDISPLAY ${P3TINYDISPLAY_HEADERS} ${P3TINYDISPLAY_SOURCES})
if (HAVE_X11)
    target_link_libraries(p3tinydisplay panda p3x11display)
# p3windisplay
# p3cocoadisplay
elseif (WASI OR EMSCRIPTEN)
    target_link_libraries(p3tinydisplay panda )
else()
    message(WARNING "no display and not headless build")
    target_link_libraries(p3tinydisplay panda )
endif()


if(NOT BUILD_METALIBS)
  install(TARGETS p3tinydisplay
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3TINYDISPLAY_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

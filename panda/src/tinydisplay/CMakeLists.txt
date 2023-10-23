if (NOT HAVE_TINYDISPLAY)
  return()
endif()

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
  srgb_tables.cxx
  zbuffer.cxx
  zdither.cxx
  zline.cxx
  zmath.cxx
)

set(P3TINYDISPLAY_ZTRIANGLE_SOURCES
  ztriangle_1.cxx
  ztriangle_2.cxx
  ztriangle_3.cxx
  ztriangle_4.cxx
  ztriangle_table.cxx
)

set_source_files_properties(${P3TINYDISPLAY_ZTRIANGLE_SOURCES}
  PROPERTIES SKIP_UNITY_BUILD_INCLUSION YES)

if(NOT MSVC)
  set_source_files_properties(${P3TINYDISPLAY_ZTRIANGLE_SOURCES}
    PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable")
endif()

if(HAVE_COCOA)
  set(P3TINYDISPLAY_HEADERS ${P3TINYDISPLAY_HEADERS}
    tinyCocoaGraphicsPipe.I tinyCocoaGraphicsPipe.h
    tinyCocoaGraphicsWindow.I tinyCocoaGraphicsWindow.h)

  set(P3TINYDISPLAY_SOURCES ${P3TINYDISPLAY_SOURCES}
    tinyCocoaGraphicsPipe.cxx
    tinyCocoaGraphicsWindow.mm)

  set_source_files_properties(tinyCocoaGraphicsWindow.mm
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION YES)

  add_compile_definitions(HAVE_COCOA)
endif()

composite_sources(p3tinydisplay P3TINYDISPLAY_SOURCES)

# Determine the additional components to link in.
set(COCOADISPLAY_LINK_TARGETS)

if(WIN32)
  list(APPEND COCOADISPLAY_LINK_TARGETS p3windisplay)
endif()

if(HAVE_X11)
  list(APPEND COCOADISPLAY_LINK_TARGETS p3x11display)
endif()

if(HAVE_COCOA)
  list(APPEND COCOADISPLAY_LINK_TARGETS p3cocoadisplay)
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "TinyDisplay")
add_metalib(p3tinydisplay ${MODULE_TYPE}
  ${P3TINYDISPLAY_HEADERS} ${P3TINYDISPLAY_SOURCES} ${P3TINYDISPLAY_ZTRIANGLE_SOURCES}
  COMPONENTS ${COCOADISPLAY_LINK_TARGETS})
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

set_target_properties(p3tinydisplay PROPERTIES DEFINE_SYMBOL BUILDING_TINYDISPLAY)
target_link_libraries(p3tinydisplay panda)

install(TARGETS p3tinydisplay
  EXPORT TinyDisplay COMPONENT TinyDisplay
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT TinyDisplayDevel)

export_targets(TinyDisplay NAMESPACE "Panda3D::TinyDisplay::" COMPONENT TinyDisplayDevel)

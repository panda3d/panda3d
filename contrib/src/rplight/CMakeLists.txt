set(P3RPLIGHT_HEADERS
  config_rplight.h
  gpuCommand.h gpuCommand.I
  gpuCommandList.h
  iesDataset.h
  internalLightManager.h internalLightManager.I
  pointerSlotStorage.h
  pssmCameraRig.h pssmCameraRig.I
  rpLight.h rpLight.I
  rpPointLight.h rpPointLight.I
  rpSpotLight.h rpSpotLight.I
  shadowAtlas.h shadowAtlas.I
  shadowManager.h shadowManager.I
  shadowSource.h shadowSource.I
  tagStateManager.h tagStateManager.I
)

set(P3RPLIGHT_SOURCES
  config_rplight.cxx
  gpuCommand.cxx
  gpuCommandList.cxx
  iesDataset.cxx
  internalLightManager.cxx
  pssmCameraRig.cxx
  rpLight.cxx
  rpPointLight.cxx
  rpSpotLight.cxx
  shadowAtlas.cxx
  shadowManager.cxx
  shadowSource.cxx
  tagStateManager.cxx
)

composite_sources(p3rplight P3RPLIGHT_SOURCES)
# STATIC, because it doesn't contain any EXPCL_ stuff
add_library(p3rplight STATIC ${P3RPLIGHT_HEADERS} ${P3RPLIGHT_SOURCES})
target_link_libraries(p3rplight p3contribbase)
target_interrogate(p3rplight ALL)

if(MODULE_TYPE STREQUAL "MODULE")
  # Because it's STATIC (see above), we need to make it PIC so it can link into
  # a Python module
  set_target_properties(p3rplight PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

install(TARGETS p3rplight
  EXPORT Contrib COMPONENT Contrib
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT ContribDevel)
install(FILES ${P3RPLIGHT_HEADERS} COMPONENT ContribDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

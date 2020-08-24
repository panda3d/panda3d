set(P3CHAN_HEADERS
  animBundle.I animBundle.h
  animBundleNode.I animBundleNode.h
  animChannel.I animChannel.h animChannelBase.I
  animChannelBase.h
  animChannelFixed.I animChannelFixed.h
  animChannelMatrixDynamic.I animChannelMatrixDynamic.h
  animChannelMatrixFixed.I animChannelMatrixFixed.h
  animChannelMatrixXfmTable.I animChannelMatrixXfmTable.h
  animChannelScalarDynamic.I animChannelScalarDynamic.h
  animChannelScalarTable.I animChannelScalarTable.h
  animControl.I
  animControl.h animControlCollection.I
  animControlCollection.h animGroup.I animGroup.h
  animPreloadTable.I animPreloadTable.h
  auto_bind.h
  bindAnimRequest.I bindAnimRequest.h
  config_chan.h
  movingPart.I movingPart.h
  movingPartBase.I movingPartBase.h
  movingPartMatrix.I movingPartMatrix.h movingPartScalar.I
  movingPartScalar.h partBundle.I partBundle.h
  partBundleHandle.I partBundleHandle.h
  partBundleNode.I partBundleNode.h
  partGroup.I partGroup.h
  partSubset.I partSubset.h
  vector_PartGroupStar.h
)

set(P3CHAN_SOURCES
  animBundle.cxx
  animBundleNode.cxx
  animChannel.cxx
  animChannelBase.cxx
  animChannelFixed.cxx
  animChannelMatrixDynamic.cxx
  animChannelMatrixFixed.cxx
  animChannelMatrixXfmTable.cxx
  animChannelScalarDynamic.cxx
  animChannelScalarTable.cxx
  animControl.cxx
  animControlCollection.cxx animGroup.cxx
  animPreloadTable.cxx
  auto_bind.cxx
  bindAnimRequest.cxx
  config_chan.cxx movingPartBase.cxx movingPartMatrix.cxx
  movingPartScalar.cxx partBundle.cxx
  partBundleHandle.cxx
  partBundleNode.cxx
  partGroup.cxx
  partSubset.cxx
  vector_PartGroupStar.cxx
)

composite_sources(p3chan P3CHAN_SOURCES)
add_component_library(p3chan NOINIT SYMBOL BUILDING_PANDA_CHAN
  ${P3CHAN_HEADERS} ${P3CHAN_SOURCES})
target_link_libraries(p3chan p3pgraph)
target_interrogate(p3chan ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3chan
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3CHAN_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

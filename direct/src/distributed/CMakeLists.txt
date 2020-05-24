if(NOT HAVE_PYTHON)
  return()
endif()

set(P3DISTRIBUTED_HEADERS
  config_distributed.h
  cConnectionRepository.h
  cConnectionRepository.I
  cDistributedSmoothNodeBase.h
  cDistributedSmoothNodeBase.I
)

set(P3DISTRIBUTED_SOURCES
  config_distributed.cxx
)

set(P3DISTRIBUTED_IGATEEXT
  cConnectionRepository.cxx
  cDistributedSmoothNodeBase.cxx
)

add_component_library(p3distributed NOINIT SYMBOL BUILDING_DIRECT_DISTRIBUTED
  ${P3DISTRIBUTED_HEADERS} ${P3DISTRIBUTED_SOURCES})
target_link_libraries(p3distributed p3directbase p3dcparser panda)
target_interrogate(p3distributed ALL EXTENSIONS ${P3DISTRIBUTED_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3distributed
    EXPORT Direct COMPONENT Direct
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT DirectDevel)
endif()
install(FILES ${P3DISTRIBUTED_HEADERS} COMPONENT DirectDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

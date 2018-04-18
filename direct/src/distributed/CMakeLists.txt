if(HAVE_PYTHON)
  set(P3DISTRIBUTED_HEADERS
      config_distributed.h
      cConnectionRepository.I
      cConnectionRepository.h
      cDistributedSmoothNodeBase.I
      cDistributedSmoothNodeBase.h)

  set(P3DISTRIBUTED_SOURCES
      config_distributed.cxx
      cConnectionRepository.cxx
      cDistributedSmoothNodeBase.cxx)

  add_library(p3distributed ${P3DISTRIBUTED_HEADERS} ${P3DISTRIBUTED_SOURCES})
  set_target_properties(p3distributed PROPERTIES DEFINE_SYMBOL BUILDING_DIRECT_DISTRIBUTED)
  target_compile_definitions(p3distributed PUBLIC WITHIN_PANDA)
  target_link_libraries(p3distributed p3directbase p3dcparser panda)
  target_use_packages(p3distributed PYTHON)
  target_interrogate(p3distributed ALL)

  install(TARGETS p3distributed DESTINATION lib)
  install(FILES ${P3DISTRIBUTED_HEADERS} DESTINATION include/panda3d)
endif()

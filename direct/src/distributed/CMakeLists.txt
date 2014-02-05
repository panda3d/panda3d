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
  set_target_properties(p3distributed PROPERTIES COMPILE_DEFINITIONS WITHIN_PANDA)
  target_link_libraries(p3distributed p3directbase p3dcparser p3pgraph)
  if(WANT_NATIVE_NET)
    target_link_libraries(p3distributed p3nativenet)
  endif()
  if(HAVE_NET)
    target_link_libraries(p3distributed p3net)
  endif()
  target_interrogate(p3distributed ALL)

  install(TARGETS p3distributed DESTINATION lib)
endif()

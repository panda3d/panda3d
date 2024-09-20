if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(pfm-bba
  config_pfmprogs.cxx config_pfmprogs.h
  pfmBba.cxx pfmBba.h)
target_link_libraries(pfm-bba p3progbase)
install(TARGETS pfm-bba EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(pfm-trans pfmTrans.cxx pfmTrans.h)
target_link_libraries(pfm-trans p3progbase)
install(TARGETS pfm-trans EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

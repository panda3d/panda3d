add_executable(pfm-bba
  config_pfmprogs.cxx config_pfmprogs.h
  pfmBba.cxx pfmBba.h)
target_link_libraries(pfm-bba p3progbase)
install(TARGETS pfm-bba DESTINATION bin)

add_executable(pfm-trans pfmTrans.cxx pfmTrans.h)
target_link_libraries(pfm-trans p3progbase)
install(TARGETS pfm-trans DESTINATION bin)

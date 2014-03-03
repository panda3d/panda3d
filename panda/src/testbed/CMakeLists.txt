add_executable(pview pview.cxx)
target_link_libraries(pview p3framework p3pystub)
install(TARGETS pview DESTINATION bin)

#begin test_bin_target
  #define TARGET pgrid

  #define SOURCES \
#    pgrid.cxx
  #define UNIX_SYS_LIBS m
#end test_bin_target

#begin test_bin_target
  #define TARGET test_lod
  #define SOURCES test_lod.cxx
#end test_bin_target

#begin test_bin_target
  #define TARGET test_texmem
  #define SOURCES test_texmem.cxx
#end test_bin_target

#begin test_bin_target
  #define TARGET test_map
  #define SOURCES test_map.cxx
#end test_bin_target

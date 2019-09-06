if(NOT BUILD_PANDATOOL)
  # It's safe to say, if the user doesn't want pandatool, they don't want these
  # tools either.
  return()
endif()

if(HAVE_OPENSSL)
  add_executable(pencrypt pencrypt.cxx)
  target_link_libraries(pencrypt panda)

  add_executable(pdecrypt pdecrypt.cxx)
  target_link_libraries(pdecrypt panda)

  install(TARGETS pencrypt pdecrypt
    EXPORT Tools COMPONENT Tools
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(HAVE_ZLIB)
  add_executable(pzip pzip.cxx)
  target_link_libraries(pzip panda)

  if(NOT CMAKE_CROSSCOMPILING)
    add_executable(host_pzip ALIAS pzip)
  endif()

  add_executable(punzip punzip.cxx)
  target_link_libraries(punzip panda)

  install(TARGETS pzip punzip
    EXPORT Tools COMPONENT Tools
    DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(multify multify.cxx)
target_link_libraries(multify panda)
install(TARGETS multify EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

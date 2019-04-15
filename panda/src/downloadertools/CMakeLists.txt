if(NOT BUILD_PANDATOOL)
  # It's safe to say, if the user doesn't want pandatool, they don't want these
  # tools either.
  return()
endif()

if(HAVE_OPENSSL)
  add_executable(apply_patch apply_patch.cxx)
  target_link_libraries(apply_patch panda)

  add_executable(build_patch build_patch.cxx)
  target_link_libraries(build_patch panda)

  add_executable(show_ddb show_ddb.cxx)
  target_link_libraries(show_ddb panda)

  add_executable(check_md5 check_md5.cxx)
  target_link_libraries(check_md5 panda)

  add_executable(pencrypt pencrypt.cxx)
  target_link_libraries(pencrypt panda)

  add_executable(pdecrypt pdecrypt.cxx)
  target_link_libraries(pdecrypt panda)

  install(TARGETS apply_patch build_patch show_ddb check_md5 pencrypt pdecrypt
    EXPORT Tools COMPONENT Tools
    DESTINATION bin)
endif()

if(HAVE_ZLIB)
  add_executable(check_adler check_adler.cxx)
  target_link_libraries(check_adler panda)

  add_executable(check_crc check_crc.cxx)
  target_link_libraries(check_crc panda)

  add_executable(pzip pzip.cxx)
  target_link_libraries(pzip panda)

  add_executable(punzip punzip.cxx)
  target_link_libraries(punzip panda)

  install(TARGETS check_adler check_crc pzip punzip
    EXPORT Tools COMPONENT Tools
    DESTINATION bin)
endif()

add_executable(multify multify.cxx)
target_link_libraries(multify panda)
install(TARGETS multify EXPORT Tools COMPONENT Tools DESTINATION bin)

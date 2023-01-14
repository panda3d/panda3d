if(NOT HAVE_PYTHON)
  return()
endif()

add_executable(deploy-stub deploy-stub.c)

if(IS_OSX)
  target_link_options(deploy-stub PRIVATE -sectcreate __PANDA __panda /dev/null)
  set_target_properties(deploy-stub PROPERTIES
    INSTALL_RPATH "@executable_path"
    BUILD_WITH_INSTALL_RPATH ON)
elseif(WIN32)
  target_sources(deploy-stub PRIVATE frozen_dllmain.c)

elseif(IS_LINUX OR IS_FREEBSD)
  set_target_properties(deploy-stub PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH ON)
  target_link_options(deploy-stub PRIVATE -Wl,--disable-new-dtags -Wl,-z,origin -rdynamic)
  target_link_libraries(deploy-stub m)
endif()

target_link_libraries(deploy-stub Python::Python)
install(TARGETS deploy-stub)

if(WIN32 OR IS_OSX)
  add_executable(deploy-stubw WIN32 deploy-stub.c)

  if(IS_OSX)
    target_link_options(deploy-stubw PRIVATE -sectcreate __PANDA __panda /dev/null)
    set_target_properties(deploy-stubw PROPERTIES
      INSTALL_RPATH "@executable_path/../Frameworks"
      BUILD_WITH_INSTALL_RPATH ON)
    target_compile_definitions(deploy-stubw PRIVATE MACOS_APP_BUNDLE=1)
  elseif(WIN32)
    target_sources(deploy-stubw PRIVATE frozen_dllmain.c)
  endif()

  target_link_libraries(deploy-stubw Python::Python)
  install(TARGETS deploy-stubw)
endif()

if(IS_OSX)
  add_executable(deploy-stube WIN32 deploy-stube.c)
  set_target_properties(deploy-stube PROPERTIES
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH ON)
  target_compile_definitions(deploy-stube PRIVATE MACOS_APP_BUNDLE=1)

  target_link_libraries(deploy-stube Python::Python)
  install(TARGETS deploy-stube)
endif()

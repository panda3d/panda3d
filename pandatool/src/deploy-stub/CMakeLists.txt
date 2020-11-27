if(NOT HAVE_PYTHON)
  return()
endif()

if(NOT IS_IOS)
  add_executable(deploy-stub deploy-stub.c)

  if(APPLE)
    target_link_options(deploy-stub PRIVATE -sectcreate __PANDA __panda /dev/null)
    set_target_properties(deploy-stub PROPERTIES
      INSTALL_RPATH "@executable_path"
      BUILD_WITH_INSTALL_RPATH ON)
    if(IS_IOS)
      target_sources(deploy-stub PRIVATE ios_main.m)
      find_library(UIKIT_LIBRARY UIKit)
      target_link_libraries(deploy-stub ${UIKIT_LIBRARY})
      target_link_options(deploy-stub PRIVATE -undefined dynamic_lookup)
    endif()
  elseif(WIN32)
    target_sources(deploy-stub PRIVATE frozen_dllmain.c)
  elseif(IS_LINUX OR IS_FREEBSD)
    set_target_properties(deploy-stub PROPERTIES
      INSTALL_RPATH "$ORIGIN"
      BUILD_WITH_INSTALL_RPATH ON)
  endif()

  target_link_libraries(deploy-stub Python::Python)
  install(TARGETS deploy-stub EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(WIN32 OR APPLE)
  add_executable(deploy-stubw deploy-stub.c)

  if(APPLE)
    if(IS_IOS)
      set(_rpath "@executable_path")
      target_sources(deploy-stubw PRIVATE ios_main.m)
    else()
      set(_rpath "@executable_path/../Frameworks")
    endif()
    target_compile_definitions(deploy-stubw PRIVATE MACOS_APP_BUNDLE=1)
    target_link_options(deploy-stubw PRIVATE -sectcreate __PANDA __panda /dev/null)

    set_target_properties(deploy-stubw PROPERTIES
      UNITY_BUILD OFF
      INSTALL_RPATH ${_rpath}
      BUILD_WITH_INSTALL_RPATH ON
      XCODE_PRODUCT_TYPE "com.apple.product-type.application")

    if(IS_IOS)
      find_library(UIKIT_LIBRARY UIKit)

      target_link_libraries(deploy-stubw ${UIKIT_LIBRARY} OpenSSL::SSL OpenSSL::Crypto p3ios)
      target_link_options(deploy-stubw PRIVATE -undefined dynamic_lookup)

      set_target_properties(deploy-stubw PROPERTIES
        XCODE_PRODUCT_TYPE "com.apple.product-type.library.static"
        XCODE_ATTRIBUTE_MACH_O_TYPE "mh_execute"
        XCODE_ATTRIBUTE_EXECUTABLE_PREFIX ""
        XCODE_ATTRIBUTE_EXECUTABLE_SUFFIX "")
    endif()
  elseif(WIN32)
    target_sources(deploy-stubw PRIVATE frozen_dllmain.c)
    set_property(TARGET deploy-stubw WIN32_EXECUTABLE)
  endif()

  target_link_libraries(deploy-stubw Python::Python)
  install(TARGETS deploy-stubw EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

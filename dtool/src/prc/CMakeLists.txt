set(P3PRC_HEADERS
  androidLogStream.h
  bigEndian.h
  config_prc.h
  configDeclaration.I configDeclaration.h
  configFlags.I configFlags.h
  configPage.I configPage.h
  configPageManager.I configPageManager.h
  configVariable.I configVariable.h
  configVariableBase.I configVariableBase.h
  configVariableBool.I configVariableBool.h
  configVariableCore.I configVariableCore.h
  configVariableDouble.I configVariableDouble.h
  configVariableEnum.I configVariableEnum.h
  configVariableFilename.I configVariableFilename.h
  configVariableInt.I configVariableInt.h
  configVariableInt64.I configVariableInt64.h
  configVariableList.I configVariableList.h
  configVariableManager.I configVariableManager.h
  configVariableSearchPath.I configVariableSearchPath.h
  configVariableString.I configVariableString.h
  encryptStreamBuf.h encryptStreamBuf.I encryptStream.h encryptStream.I
  littleEndian.h
  nativeNumericData.I nativeNumericData.h
  pnotify.I pnotify.h
  notifyCategory.I notifyCategory.h
  notifyCategoryProxy.I notifyCategoryProxy.h
  notifySeverity.h
  prcKeyRegistry.h prcKeyRegistry.I
  reversedNumericData.I reversedNumericData.h
  streamReader.I streamReader.h
  streamWrapper.I streamWrapper.h
  streamWriter.I streamWriter.h
  ${CMAKE_CURRENT_BINARY_DIR}/prc_parameters.h
)

set(P3PRC_SOURCES
  config_prc.cxx
  configDeclaration.cxx
  configFlags.cxx
  configPage.cxx
  configPageManager.cxx
  configVariable.cxx
  configVariableBase.cxx
  configVariableBool.cxx
  configVariableCore.cxx
  configVariableDouble.cxx
  configVariableEnum.cxx
  configVariableFilename.cxx
  configVariableInt.cxx
  configVariableInt64.cxx
  configVariableList.cxx
  configVariableManager.cxx
  configVariableSearchPath.cxx
  configVariableString.cxx
  nativeNumericData.cxx
  notify.cxx
  notifyCategory.cxx
  notifySeverity.cxx
  prcKeyRegistry.cxx
  reversedNumericData.cxx
  streamReader.cxx streamWrapper.cxx streamWriter.cxx
)

configure_file(prc_parameters.h.in prc_parameters.h)

if(ANDROID)
  set(P3PRC_SOURCES ${P3PRC_SOURCES}
    androidLogStream.cxx)
endif()

if(EMSCRIPTEN)
  set(P3PRC_SOURCES ${P3PRC_SOURCES}
    emscriptenLogStream.cxx)
endif()

if(HAVE_OPENSSL)
  list(APPEND P3PRC_SOURCES encryptStreamBuf.cxx encryptStream.cxx)
endif()

set(P3PRC_IGATEEXT
  configVariable_ext.cxx
  configVariable_ext.h
  notify_ext.cxx
  notify_ext.h
  streamReader_ext.cxx
  streamReader_ext.h
  streamWriter_ext.cxx
  streamWriter_ext.h
)

composite_sources(p3prc P3PRC_SOURCES)
add_library(p3prc
  ${P3PRC_HEADERS} ${P3PRC_SOURCES})
set_target_properties(p3prc PROPERTIES DEFINE_SYMBOL BUILDING_DTOOL_PRC)
target_include_directories(p3prc PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
target_link_libraries(p3prc p3dtool PKG::OPENSSL)
target_interrogate(p3prc ALL EXTENSIONS ${P3PRC_IGATEEXT})

if(ANDROID)
  target_link_libraries(p3prc log)
endif()

if(HAVE_OPENSSL AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Do not re-export symbols from these libraries.
  target_link_options(p3prc PRIVATE "LINKER:--exclude-libs,libssl.a")
  target_link_options(p3prc PRIVATE "LINKER:--exclude-libs,libcrypto.a")
endif()

install(TARGETS p3prc
  EXPORT Core COMPONENT Core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT CoreDevel)

install(FILES ${P3PRC_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

# Also install under the old name "p3dtoolconfig"
# TODO: Remove this once "p3dtoolconfig" is deprecated
if(WIN32)
  # Symlinks aren't pretty under WIN32
  set(_symlink "copy")
else()
  set(_symlink "create_symlink")
endif()

get_target_property(_type p3prc TYPE)
if(_type STREQUAL "SHARED_LIBRARY")

  set(_suffix "${CMAKE_SHARED_LIBRARY_SUFFIX}")
  if(UNIX)
    get_target_property(_soversion p3prc SOVERSION)
    if(APPLE)
      set(_suffix ".${_soversion}${_suffix}")
    else()
      set(_suffix "${_suffix}.${_soversion}")
    endif()

    set(_file_genex "TARGET_SONAME_FILE")
    set(_install_dir "${CMAKE_INSTALL_LIBDIR}")

  else()
    # Win32?
    set(_file_genex "TARGET_FILE")
    set(_install_dir "${CMAKE_INSTALL_BINDIR}")

  endif()

  set(_soname "${CMAKE_SHARED_LIBRARY_PREFIX}p3dtoolconfig${_suffix}")

  add_custom_command(TARGET p3prc POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E chdir "$<${_file_genex}_DIR:p3prc>"
      "${CMAKE_COMMAND}" -E "${_symlink}"
      "$<${_file_genex}_NAME:p3prc>" "${_soname}"
  )
  install(FILES "$<${_file_genex}_DIR:p3prc>/${_soname}"
    DESTINATION "${_install_dir}")
endif()

if(WIN32)
  set(_libname "${CMAKE_IMPORT_LIBRARY_PREFIX}p3dtoolconfig${CMAKE_IMPORT_LIBRARY_SUFFIX}")
else()
  set(_libname "${CMAKE_${_type}_PREFIX}p3dtoolconfig${CMAKE_${_type}_SUFFIX}")
endif()
add_custom_command(TARGET p3prc POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E chdir "$<TARGET_LINKER_FILE_DIR:p3prc>"
    "${CMAKE_COMMAND}" -E "${_symlink}"
    "$<TARGET_LINKER_FILE_NAME:p3prc>" "${_libname}"
)
install(FILES "$<TARGET_LINKER_FILE_DIR:p3prc>/${_libname}"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}")

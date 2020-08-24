if(NOT HAVE_DX9)
  return()
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "DX9Devel")
add_metalib(pandadx9 ${MODULE_TYPE}
  INCLUDE wdxGraphicsPipe9.h
  INIT init_libpandadx9 pandadx9.h
  EXPORT int get_pipe_type_pandadx9 "wdxGraphicsPipe9::get_class_type().get_index()"
  COMPONENTS p3dxgsg9 p3windisplay)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandadx9
  EXPORT DX9 COMPONENT DX9
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT DX9Devel)

export_targets(DX9 COMPONENT DX9Devel)

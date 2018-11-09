if(NOT HAVE_DX9)
  return()
endif()

add_metalib(pandadx9 ${MODULE_TYPE}
  INCLUDE wdxGraphicsPipe9.h
  INIT init_libpandadx9 pandadx9.h
  EXPORT int get_pipe_type_pandadx9 "wdxGraphicsPipe9::get_class_type().get_index()"
  COMPONENTS p3dxgsg9 p3windisplay)

install(TARGETS pandadx9 DESTINATION ${MODULE_DESTINATION})

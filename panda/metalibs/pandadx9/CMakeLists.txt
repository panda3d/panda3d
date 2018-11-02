if(NOT HAVE_DX9)
  return()
endif()

add_metalib(pandadx9 ${MODULE_TYPE} pandadx9.cxx
  COMPONENTS p3dxgsg9)
set_target_properties(pandadx9 PROPERTIES DEFINE_SYMBOL BUILDING_PANDADX)

install(TARGETS pandadx9 DESTINATION ${MODULE_DESTINATION})

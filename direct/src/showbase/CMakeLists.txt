set(P3SHOWBASE_FILES
    showBase.cxx
    showBase.h)

if(APPLE)
  list(APPEND P3SHOWBASE_FILES showBase_assist.mm)
endif()

add_component_library(p3showbase NOINIT SYMBOL BUILDING_DIRECT_SHOWBASE
  ${P3SHOWBASE_FILES})

if(APPLE)
  set_target_properties(p3showbase PROPERTIES COMPILE_FLAGS "-ObjC++")
endif()

target_link_libraries(p3showbase p3directbase panda)
target_interrogate(p3showbase ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3showbase DESTINATION lib RUNTIME DESTINATION bin)
endif()

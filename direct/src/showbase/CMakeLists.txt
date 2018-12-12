set(P3SHOWBASE_HEADERS
  showBase.h
)

set(P3SHOWBASE_SOURCES
  showBase.cxx
)

if(APPLE)
  list(APPEND P3SHOWBASE_SOURCES showBase_assist.mm)
endif()

add_component_library(p3showbase NOINIT SYMBOL BUILDING_DIRECT_SHOWBASE
  ${P3SHOWBASE_HEADERS} ${P3SHOWBASE_SOURCES})

if(APPLE)
  set_target_properties(p3showbase PROPERTIES COMPILE_FLAGS "-ObjC++")
endif()

target_link_libraries(p3showbase p3directbase panda)
target_interrogate(p3showbase ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3showbase
    EXPORT Direct COMPONENT Direct
    DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE COMPONENT DirectDevel)
endif()
install(FILES ${P3SHOWBASE_HEADERS} COMPONENT DirectDevel DESTINATION include/panda3d)

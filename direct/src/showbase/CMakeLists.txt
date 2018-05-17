set(P3SHOWBASE_FILES
    showBase.cxx
    showBase.h)
set(P3SHOWBASE_EXTRA_ARGS "")
if(IS_OSX) 
  list(APPEND P3SHOWBASE_FILES showBase_assist.mm)
  set(P3SHOWBASE_EXTRA_ARGS "-ObjC++")
endif()

add_component_library(p3showbase SYMBOL BUILDING_DIRECT_SHOWBASE ${P3SHOWBASE_FILES})
set_target_properties(p3showbase PROPERTIES COMPILE_FLAGS ${P3SHOWBASE_EXTRA_ARGS})
target_link_libraries(p3showbase p3directbase panda)
target_interrogate(p3showbase ALL)

install(TARGETS p3showbase DESTINATION lib)

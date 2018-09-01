set(P3CHAR_HEADERS
  character.I character.h
  characterJoint.I characterJoint.h
  characterJointBundle.I characterJointBundle.h
  characterJointEffect.h characterJointEffect.I
  characterSlider.h
  characterVertexSlider.I characterVertexSlider.h
  config_char.h
  jointVertexTransform.I jointVertexTransform.h
)
set(P3CHAR_SOURCES
  character.cxx
  characterJoint.cxx characterJointBundle.cxx
  characterJointEffect.cxx
  characterSlider.cxx
  characterVertexSlider.cxx
  config_char.cxx
  jointVertexTransform.cxx
)

composite_sources(p3char P3CHAR_SOURCES)
add_component_library(p3char SYMBOL BUILDING_PANDA_CHAR
  ${P3CHAR_HEADERS} ${P3CHAR_SOURCES})
target_link_libraries(p3char p3chan)
target_interrogate(p3char ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3char DESTINATION lib RUNTIME DESTINATION bin)
endif()
install(FILES ${P3CHAR_HEADERS} DESTINATION include/panda3d)

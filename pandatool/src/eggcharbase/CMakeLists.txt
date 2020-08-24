if(NOT HAVE_EGG)
  return()
endif()

set(P3EGGCHARBASE_HEADERS
  config_eggcharbase.h
  eggBackPointer.h
  eggCharacterCollection.h eggCharacterCollection.I
  eggCharacterData.h eggCharacterData.I
  eggCharacterDb.h eggCharacterDb.I
  eggCharacterFilter.h
  eggComponentData.h eggComponentData.I
  eggJointData.h eggJointData.I
  eggJointNodePointer.h
  eggJointPointer.h eggJointPointer.I
  eggMatrixTablePointer.h
  eggScalarTablePointer.h
  eggSliderData.h eggSliderData.I
  eggSliderPointer.h
  eggVertexPointer.h
)

set(P3EGGCHARBASE_SOURCES
  config_eggcharbase.cxx
  eggBackPointer.cxx
  eggCharacterCollection.cxx
  eggCharacterData.cxx
  eggCharacterDb.cxx
  eggCharacterFilter.cxx
  eggComponentData.cxx
  eggJointData.cxx
  eggJointNodePointer.cxx
  eggJointPointer.cxx
  eggMatrixTablePointer.cxx
  eggScalarTablePointer.cxx
  eggSliderData.cxx
  eggSliderPointer.cxx
  eggVertexPointer.cxx
)

composite_sources(p3eggcharbase P3EGGCHARBASE_SOURCES)
add_library(p3eggcharbase STATIC ${P3EGGCHARBASE_HEADERS} ${P3EGGCHARBASE_SOURCES})
target_link_libraries(p3eggcharbase p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

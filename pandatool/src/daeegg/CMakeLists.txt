if(NOT HAVE_EGG OR NOT HAVE_FCOLLADA)
  return()
endif()

set(P3DAEEGG_HEADERS
  config_daeegg.h
  daeCharacter.h
  daeMaterials.h
  daeToEggConverter.h
  fcollada_utils.h
  pre_fcollada_include.h
)

set(P3DAEEGG_SOURCES
  config_daeegg.cxx
  daeCharacter.cxx
  daeMaterials.cxx
  daeToEggConverter.cxx
)

add_library(p3daeegg STATIC ${P3DAEEGG_HEADERS} ${P3DAEEGG_SOURCES})
target_link_libraries(p3daeegg p3eggbase
  PKG::FCOLLADA)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

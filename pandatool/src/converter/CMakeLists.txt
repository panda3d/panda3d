if(NOT HAVE_EGG)
  return()
endif()

set(P3CONVERTER_HEADERS
  somethingToEggConverter.h somethingToEggConverter.I
  eggToSomethingConverter.h eggToSomethingConverter.I
)

set(P3CONVERTER_SOURCES
  somethingToEggConverter.cxx eggToSomethingConverter.cxx
)

add_library(p3converter STATIC ${P3CONVERTER_HEADERS} ${P3CONVERTER_SOURCES})
target_link_libraries(p3converter p3pandatoolbase pandaegg)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

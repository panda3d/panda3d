if(NOT HAVE_EGG)
  return()
endif()

set(P3EGGBASE_HEADERS
  eggBase.h eggConverter.h eggFilter.h
  eggMakeSomething.h
  eggMultiBase.h eggMultiFilter.h
  eggReader.h eggSingleBase.h
  eggToSomething.h eggWriter.h
  somethingToEgg.h
)

set(P3EGGBASE_SOURCES
  eggBase.cxx eggConverter.cxx eggFilter.cxx
  eggMakeSomething.cxx
  eggMultiBase.cxx
  eggMultiFilter.cxx eggReader.cxx eggSingleBase.cxx
  eggToSomething.cxx
  eggWriter.cxx somethingToEgg.cxx
)

composite_sources(p3eggbase P3EGGBASE_SOURCES)
add_library(p3eggbase STATIC ${P3EGGBASE_HEADERS} ${P3EGGBASE_SOURCES})
target_link_libraries(p3eggbase p3progbase p3converter)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

set(P3IMAGEBASE_HEADERS
  imageBase.h
  imageFilter.h
  imageReader.h
  imageWriter.h imageWriter.I
)

set(P3IMAGEBASE_SOURCES
  imageBase.cxx
  imageFilter.cxx
  imageReader.cxx
  imageWriter.cxx
)

composite_sources(p3imagebase P3IMAGEBASE_SOURCES)
add_library(p3imagebase STATIC ${P3IMAGEBASE_HEADERS} ${P3IMAGEBASE_SOURCES})
target_link_libraries(p3imagebase p3progbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

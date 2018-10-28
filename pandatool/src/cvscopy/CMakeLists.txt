set(P3CVSCOPY_HEADERS
  cvsCopy.h cvsSourceDirectory.h cvsSourceTree.h
)

set(P3CVSCOPY_SOURCES
  cvsCopy.cxx cvsSourceDirectory.cxx cvsSourceTree.cxx
)

composite_sources(p3cvscopy P3CVSCOPY_SOURCES)
add_library(p3cvscopy STATIC ${P3CVSCOPY_HEADERS} ${P3CVSCOPY_SOURCES})
target_link_libraries(p3cvscopy p3progbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

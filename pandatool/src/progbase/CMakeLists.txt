set(P3PROGBASE_HEADERS
    programBase.I programBase.h
    withOutputFile.I withOutputFile.h
    wordWrapStream.h wordWrapStreamBuf.I
    wordWrapStreamBuf.h)

set(P3PROGBASE_SOURCES
    programBase.cxx withOutputFile.cxx wordWrapStream.cxx
    wordWrapStreamBuf.cxx)

composite_sources(p3progbase P3PROGBASE_SOURCES)
add_library(p3progbase STATIC ${P3PROGBASE_HEADERS} ${P3PROGBASE_SOURCES})
target_link_libraries(p3progbase p3pandatoolbase panda)
target_use_packages(p3progbase ZLIB)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

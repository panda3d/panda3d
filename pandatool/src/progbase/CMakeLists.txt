set(P3PROGBASE_HEADERS
  programBase.h programBase.I
  withOutputFile.h withOutputFile.I
  wordWrapStream.h wordWrapStreamBuf.I
  wordWrapStreamBuf.h
)

set(P3PROGBASE_SOURCES
  programBase.cxx withOutputFile.cxx wordWrapStream.cxx
  wordWrapStreamBuf.cxx
)

composite_sources(p3progbase P3PROGBASE_SOURCES)
add_library(p3progbase STATIC ${P3PROGBASE_HEADERS} ${P3PROGBASE_SOURCES})
target_link_libraries(p3progbase p3pandatoolbase panda PKG::ZLIB)

if(IOCTL_TERMINAL_WIDTH)
  target_compile_definitions(p3progbase PRIVATE IOCTL_TERMINAL_WIDTH)
endif()

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

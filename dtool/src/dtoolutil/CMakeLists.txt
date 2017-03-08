configure_file(pandaVersion.h.in pandaVersion.h)
configure_file(checkPandaVersion.h.in checkPandaVersion.h)
configure_file(checkPandaVersion.cxx.in checkPandaVersion.cxx)

set(P3DTOOLUTIL_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/checkPandaVersion.h
  ${CMAKE_CURRENT_BINARY_DIR}/pandaVersion.h
  config_dtoolutil.h
  dSearchPath.I dSearchPath.h
  executionEnvironment.I executionEnvironment.h filename.I
  filename.h
  globPattern.I globPattern.h
  lineStream.I lineStream.h
  lineStreamBuf.I lineStreamBuf.h
  load_dso.h
  pandaFileStream.h pandaFileStream.I
  pandaFileStreamBuf.h
  pandaSystem.h
  panda_getopt.h panda_getopt_long.h panda_getopt_impl.h
  pfstream.h pfstream.I pfstreamBuf.h
  preprocess_argv.h
  string_utils.h string_utils.I
  stringDecoder.h stringDecoder.I
  textEncoder.h textEncoder.I
  unicodeLatinMap.h
  vector_string.h
  vector_src.h
  win32ArgParser.h
)

if(APPLE)
  set(P3DTOOLUTIL_HEADERS ${P3DTOOLUTIL_HEADERS}
    filename_assist.mm filename_assist.h)
endif()

set(P3DTOOLUTIL_SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/checkPandaVersion.cxx
  config_dtoolutil.cxx
  dSearchPath.cxx
  executionEnvironment.cxx filename.cxx
  globPattern.cxx
  lineStream.cxx lineStreamBuf.cxx
  load_dso.cxx
  pandaFileStream.cxx pandaFileStreamBuf.cxx
  pandaSystem.cxx
  panda_getopt_impl.cxx
  pfstreamBuf.cxx pfstream.cxx
  preprocess_argv.cxx
  string_utils.cxx
  stringDecoder.cxx
  textEncoder.cxx
  unicodeLatinMap.cxx
  vector_string.cxx
  win32ArgParser.cxx
)

composite_sources(p3dtoolutil P3DTOOLUTIL_SOURCES)

add_library(p3dtoolutil ${P3DTOOLUTIL_HEADERS} ${P3DTOOLUTIL_SOURCES})
target_link_libraries(p3dtoolutil p3dtoolbase ${CMAKE_DL_LIBS})

if(APPLE)
  find_library(FOUNDATION_LIBRARY Foundation)
  find_library(APPKIT_LIBRARY AppKit)
  target_link_libraries(p3dtoolutil ${FOUNDATION_LIBRARY} ${APPKIT_LIBRARY})
endif()

install(TARGETS p3dtoolutil DESTINATION lib)
install(FILES ${P3DTOOLUTIL_HEADERS} DESTINATION include/panda3d)

set(P3DTOOLUTIL_HEADERS
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
  vector_double.h
  vector_float.h
  vector_int.h
  vector_string.h
  vector_uchar.h
  vector_src.h
  win32ArgParser.h
)

if(APPLE)
  set(P3DTOOLUTIL_HEADERS ${P3DTOOLUTIL_HEADERS}
    filename_assist.mm filename_assist.h)
  set_source_files_properties(filename_assist.mm filename_assist.h PROPERTIES WRAP_EXCLUDE YES)
endif()

set(P3DTOOLUTIL_SOURCES
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
  vector_double.cxx
  vector_float.cxx
  vector_int.cxx
  vector_string.cxx
  vector_uchar.cxx
  win32ArgParser.cxx
)

set(P3DTOOLUTIL_IGATEEXT
  filename_ext.cxx
  filename_ext.h
  globPattern_ext.cxx
  globPattern_ext.h
  textEncoder_ext.cxx
  textEncoder_ext.h
)

composite_sources(p3dtoolutil P3DTOOLUTIL_SOURCES)

add_component_library(p3dtoolutil SYMBOL BUILDING_DTOOL_DTOOLUTIL
  ${P3DTOOLUTIL_HEADERS} ${P3DTOOLUTIL_SOURCES})
target_link_libraries(p3dtoolutil p3dtoolbase ${CMAKE_DL_LIBS})
target_interrogate(p3dtoolutil ALL EXTENSIONS ${P3DTOOLUTIL_IGATEEXT})

if(APPLE)
  set_target_properties(p3dtoolutil PROPERTIES COMPILE_FLAGS "-ObjC++")
  find_library(FOUNDATION_LIBRARY Foundation)
  find_library(APPKIT_LIBRARY AppKit)
  target_link_libraries(p3dtoolutil ${FOUNDATION_LIBRARY} ${APPKIT_LIBRARY})
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3dtoolutil DESTINATION lib RUNTIME DESTINATION bin)
endif()
install(FILES ${P3DTOOLUTIL_HEADERS} DESTINATION include/panda3d)

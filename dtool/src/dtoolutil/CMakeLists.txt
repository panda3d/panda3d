include_directories(../dtoolbase)

configure_file(pandaVersion.h.cmake
               pandaVersion.h)

configure_file(checkPandaVersion.h.cmake
               checkPandaVersion.h)

configure_file(checkPandaVersion.cxx.cmake
               checkPandaVersion.cxx)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(P3DTOOLUTIL_HEADERS
	${CMAKE_CURRENT_BINARY_DIR}/checkPandaVersion.h
	config_dtoolutil.h
	dSearchPath.I dSearchPath.h
	executionEnvironment.I executionEnvironment.h filename.I 
	filename.h
	globPattern.I globPattern.h
	load_dso.h
	pandaFileStream.h pandaFileStream.I
	pandaFileStreamBuf.h
	pandaSystem.h pandaVersion.h
	panda_getopt.h panda_getopt_long.h panda_getopt_impl.h
	pfstream.h pfstream.I pfstreamBuf.h
	preprocess_argv.h
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
	load_dso.cxx 
	pandaFileStream.cxx pandaFileStreamBuf.cxx
	pandaSystem.cxx
	panda_getopt_impl.cxx
	pfstreamBuf.cxx pfstream.cxx
	preprocess_argv.cxx
	stringDecoder.cxx
	textEncoder.cxx
	unicodeLatinMap.cxx
	vector_string.cxx
	win32ArgParser.cxx
)

add_library(p3dtoolutil STATIC
	${P3DTOOLUTIL_HEADERS}
	${P3DTOOLUTIL_SOURCES}
)

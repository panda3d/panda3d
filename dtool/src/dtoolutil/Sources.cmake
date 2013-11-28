include_directories(${CMAKE_CURRENT_LIST_DIR}/../dtoolbase)

configure_file(${CMAKE_CURRENT_LIST_DIR}/pandaVersion.h.cmake
               ${CMAKE_CURRENT_LIST_DIR}/pandaVersion.h)

configure_file(${CMAKE_CURRENT_LIST_DIR}/checkPandaVersion.h.cmake
               ${CMAKE_CURRENT_LIST_DIR}/checkPandaVersion.h)

configure_file(${CMAKE_CURRENT_LIST_DIR}/checkPandaVersion.cxx.cmake
               ${CMAKE_CURRENT_LIST_DIR}/checkPandaVersion.cxx)

set(P3DTOOLUTIL_HEADER_FILES
	checkPandaVersion.h
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
	set(P3DTOOL_HEADER_FILES ${P3DTOOL_HEADER_FILES}
		filename_assist.mm filename_assist.h)
endif()

set(P3DTOOLUTIL_SOURCE_FILES
	checkPandaVersion.cxx
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

foreach(FILENAME IN LISTS P3DTOOLUTIL_HEADER_FILES)
	set(P3DTOOLUTIL_INSTALL_HEADERS ${P3DTOOLUTIL_INSTALL_HEADERS} ${CMAKE_CURRENT_LIST_DIR}/${FILENAME})
endforeach()
foreach(FILENAME IN LISTS P3DTOOLUTIL_SOURCE_FILES)
	set(P3DTOOLUTIL_SOURCES ${P3DTOOLUTIL_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/${FILENAME})
endforeach()

add_library(p3dtoolutil
	${P3DTOOLUTIL_INSTALL_HEADERS}
	${P3DTOOLUTIL_SOURCES}
)

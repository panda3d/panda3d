set(P3INTERROGATEDB_HEADERS
	config_interrogatedb.h indexRemapper.h interrogateComponent.I
	interrogateComponent.h interrogateDatabase.I
	interrogateDatabase.h interrogateElement.I
	interrogateElement.h interrogateFunction.I
	interrogateFunction.h interrogateFunctionWrapper.I
	interrogateFunctionWrapper.h
	interrogateMakeSeq.I interrogateMakeSeq.h
	interrogateManifest.I interrogateManifest.h
	interrogateType.I interrogateType.h
	interrogate_datafile.I interrogate_datafile.h
	interrogate_interface.h interrogate_request.h
	extension.h py_compat.h py_panda.h py_wrappers.h)

set(P3INTERROGATEDB_SOURCES
	config_interrogatedb.cxx
	dtool_super_base.cxx
	indexRemapper.cxx
	interrogateComponent.cxx interrogateDatabase.cxx
	interrogateElement.cxx interrogateFunction.cxx
	interrogateFunctionWrapper.cxx
	interrogateMakeSeq.cxx
	interrogateManifest.cxx
	interrogateType.cxx interrogate_datafile.cxx
	interrogate_interface.cxx interrogate_request.cxx
	py_compat.cxx
	py_panda.cxx
	py_wrappers.cxx)

composite_sources(p3interrogatedb P3INTERROGATEDB_SOURCES)

set(P3INTERROGATEDB_IGATE_SOURCES
	config_interrogatedb.cxx
	indexRemapper.cxx
	interrogateComponent.cxx interrogateDatabase.cxx
	interrogateElement.cxx interrogateFunction.cxx
	interrogateFunctionWrapper.cxx
	interrogateMakeSeq.cxx
	interrogateManifest.cxx
	interrogateType.cxx interrogate_datafile.cxx
	interrogate_interface.cxx interrogate_request.cxx)

add_library(p3interrogatedb ${P3INTERROGATEDB_HEADERS} ${P3INTERROGATEDB_SOURCES})
target_link_libraries(p3interrogatedb p3dconfig)
target_use_packages(p3interrogatedb PYTHON)
target_interrogate(p3interrogatedb ${P3INTERROGATEDB_IGATE_SOURCES})

install(TARGETS p3interrogatedb DESTINATION lib)
install(FILES ${P3INTERROGATEDB_HEADERS} DESTINATION include/panda3d)

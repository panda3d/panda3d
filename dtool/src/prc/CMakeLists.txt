configure_file(prc_parameters.h.in prc_parameters.h)

set(P3PRC_HEADERS
	bigEndian.h
	config_prc.h
	configDeclaration.I configDeclaration.h
	configFlags.I configFlags.h
	configPage.I configPage.h
	configPageManager.I configPageManager.h
	configVariable.I configVariable.h
	configVariableBase.I configVariableBase.h
	configVariableBool.I configVariableBool.h
	configVariableCore.I configVariableCore.h
	configVariableDouble.I configVariableDouble.h
	configVariableEnum.I configVariableEnum.h
	configVariableFilename.I configVariableFilename.h
	configVariableInt.I configVariableInt.h
	configVariableInt64.I configVariableInt64.h
	configVariableList.I configVariableList.h
	configVariableManager.I configVariableManager.h
	configVariableSearchPath.I configVariableSearchPath.h
	configVariableString.I configVariableString.h
	encryptStreamBuf.h encryptStreamBuf.I encryptStream.h encryptStream.I
	littleEndian.h
	nativeNumericData.I nativeNumericData.h
	pnotify.I pnotify.h
	notifyCategory.I notifyCategory.h
	notifyCategoryProxy.I notifyCategoryProxy.h
	notifySeverity.h
	prcKeyRegistry.h
	reversedNumericData.I reversedNumericData.h
	streamReader.I streamReader.h
	streamWrapper.I streamWrapper.h
	streamWriter.I streamWriter.h
	${CMAKE_CURRENT_BINARY_DIR}/prc_parameters.h)

set(P3PRC_SOURCES
	config_prc.cxx
	configDeclaration.cxx
	configFlags.cxx
	configPage.cxx
	configPageManager.cxx
	configVariable.cxx
	configVariableBase.cxx
	configVariableBool.cxx
	configVariableCore.cxx
	configVariableDouble.cxx
	configVariableEnum.cxx
	configVariableFilename.cxx
	configVariableInt.cxx
	configVariableInt64.cxx
	configVariableList.cxx
	configVariableManager.cxx
	configVariableSearchPath.cxx
	configVariableString.cxx
	encryptStreamBuf.cxx encryptStream.cxx
	nativeNumericData.cxx
	notify.cxx
	notifyCategory.cxx
	notifySeverity.cxx
	prcKeyRegistry.cxx
	reversedNumericData.cxx
	streamReader.cxx streamWrapper.cxx streamWriter.cxx)

composite_sources(p3prc P3PRC_SOURCES)

add_library(p3prc ${P3PRC_HEADERS} ${P3PRC_SOURCES})
target_link_libraries(p3prc p3dtoolutil p3dtoolbase ${_OPENSSL_LIBRARIES})

install(TARGETS p3prc DESTINATION lib)

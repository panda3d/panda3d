include_directories(../dtoolutil)
include_directories(../dtoolbase)

set(DEFAULT_PRC_DIR "${INSTALL_DIR}/etc" CACHE STRING "Panda uses prc files for runtime configuration. Panda will search the default .prc directory if the PRC_PATH and PRC_DIR environment variables are not defined.")
set(PRC_DCONFIG_TRUST_LEVEL 0 CACHE STRING "The trust level value for any legacy (DConfig) variables.")
set(PRC_INC_TRUST_LEVEL 0 CACHE STRING "The amount by which we globally increment the trust level.")

configure_file(prc_parameters.h.cmake
               prc_parameters.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

add_library(p3prc STATIC
	${P3PRC_HEADERS}
	${P3PRC_SOURCES})

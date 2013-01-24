#define LOCAL_LIBS p3dtoolutil p3dtoolbase
#define USE_PACKAGES openssl

#begin lib_target
  #define TARGET p3prc

  #define ANDROID_SYS_LIBS log

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx
  
  #define SOURCES \
    bigEndian.h \
    config_prc.h \
    configDeclaration.I configDeclaration.h \
    configFlags.I configFlags.h \
    configPage.I configPage.h \
    configPageManager.I configPageManager.h \
    configVariable.I configVariable.h \
    configVariableBase.I configVariableBase.h \
    configVariableBool.I configVariableBool.h \
    configVariableCore.I configVariableCore.h \
    configVariableDouble.I configVariableDouble.h \
    configVariableEnum.I configVariableEnum.h \
    configVariableFilename.I configVariableFilename.h \
    configVariableInt.I configVariableInt.h \
    configVariableInt64.I configVariableInt64.h \
    configVariableList.I configVariableList.h \
    configVariableManager.I configVariableManager.h \
    configVariableSearchPath.I configVariableSearchPath.h \
    configVariableString.I configVariableString.h \
    encryptStreamBuf.h encryptStreamBuf.I encryptStream.h encryptStream.I \
    littleEndian.h \
    nativeNumericData.I nativeNumericData.h \
    pnotify.I pnotify.h \
    notifyCategory.I notifyCategory.h \
    notifyCategoryProxy.I notifyCategoryProxy.h \
    notifySeverity.h \
    prcKeyRegistry.h \
    reversedNumericData.I reversedNumericData.h \
    streamReader.I streamReader.h \
    streamWrapper.I streamWrapper.h \
    streamWriter.I streamWriter.h

  // A generated file
  #define SOURCES $[SOURCES] prc_parameters.h 
  
  #define INCLUDED_SOURCES \
    config_prc.cxx \
    configDeclaration.cxx \
    configFlags.cxx \
    configPage.cxx \
    configPageManager.cxx \
    configVariable.cxx \
    configVariableBase.cxx \
    configVariableBool.cxx \
    configVariableCore.cxx \
    configVariableDouble.cxx \
    configVariableEnum.cxx \
    configVariableFilename.cxx \
    configVariableInt.cxx \
    configVariableInt64.cxx \
    configVariableList.cxx \
    configVariableManager.cxx \
    configVariableSearchPath.cxx \
    configVariableString.cxx \
    encryptStreamBuf.cxx encryptStream.cxx \
    nativeNumericData.cxx \
    notify.cxx \
    notifyCategory.cxx \
    notifySeverity.cxx \
    prcKeyRegistry.cxx \
    reversedNumericData.cxx \
    streamReader.cxx streamWrapper.cxx streamWriter.cxx
  
  #define INSTALL_HEADERS \
    bigEndian.h \
    config_prc.h \
    configDeclaration.I configDeclaration.h \
    configFlags.I configFlags.h \
    configPage.I configPage.h \
    configPageManager.I configPageManager.h \
    configVariable.I configVariable.h \
    configVariableBase.I configVariableBase.h \
    configVariableBool.I configVariableBool.h \
    configVariableCore.I configVariableCore.h \
    configVariableDouble.I configVariableDouble.h \
    configVariableEnum.I configVariableEnum.h \
    configVariableFilename.I configVariableFilename.h \
    configVariableInt.I configVariableInt.h \
    configVariableInt64.I configVariableInt64.h \
    configVariableList.I configVariableList.h \
    configVariableManager.I configVariableManager.h \
    configVariableSearchPath.I configVariableSearchPath.h \
    configVariableString.I configVariableString.h \
    encryptStreamBuf.h encryptStreamBuf.I encryptStream.h encryptStream.I \
    littleEndian.h \
    nativeNumericData.I nativeNumericData.h \
    pnotify.I pnotify.h \
    notifyCategory.I notifyCategory.h \
    notifyCategoryProxy.I notifyCategoryProxy.h \
    notifySeverity.h \
    prcKeyRegistry.I prcKeyRegistry.h \
    reversedNumericData.I reversedNumericData.h \
    streamReader.I streamReader.h \
    streamWrapper.I streamWrapper.h \
    streamWriter.I streamWriter.h


#end lib_target

#include $[THISDIRPREFIX]prc_parameters.h.pp


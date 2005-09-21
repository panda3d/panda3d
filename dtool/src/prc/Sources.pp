#define LOCAL_LIBS dtoolutil dtoolbase
#define USE_PACKAGES openssl

#begin lib_target
  #define TARGET prc

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx
  
  #define SOURCES \
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
    configVariableList.I configVariableList.h \
    configVariableManager.I configVariableManager.h \
    configVariableSearchPath.I configVariableSearchPath.h \
    configVariableString.I configVariableString.h \
    globPattern.I globPattern.h \
    notify.I notify.h \
    notifyCategory.I notifyCategory.h \
    notifyCategoryProxy.I notifyCategoryProxy.h \
    notifySeverity.h \
    prcKeyRegistry.h
  
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
    configVariableList.cxx \
    configVariableManager.cxx \
    configVariableSearchPath.cxx \
    configVariableString.cxx \
    globPattern.cxx \
    notify.cxx \
    notifyCategory.cxx \
    notifySeverity.cxx \
    prcKeyRegistry.cxx
  
  #define INSTALL_HEADERS \
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
    configVariableList.I configVariableList.h \
    configVariableManager.I configVariableManager.h \
    configVariableSearchPath.I configVariableSearchPath.h \
    configVariableString.I configVariableString.h \
    globPattern.I globPattern.h \
    notify.I notify.h \
    notifyCategory.I notifyCategory.h \
    notifyCategoryProxy.I notifyCategoryProxy.h \
    notifySeverity.h \
    prcKeyRegistry.I prcKeyRegistry.h

#end lib_target

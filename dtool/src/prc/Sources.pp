#define LOCAL_LIBS dtoolutil dtoolbase
#define USE_PACKAGES ssl

#begin lib_target
  #define TARGET prc
  
  #define SOURCES \
    config_prc.cxx config_prc.h \
    configDeclaration.cxx configDeclaration.I configDeclaration.h \
    configFlags.cxx configFlags.I configFlags.h \
    configPage.cxx configPage.I configPage.h \
    configPageManager.cxx configPageManager.I configPageManager.h \
    configVariable.cxx configVariable.I configVariable.h \
    configVariableBase.cxx configVariableBase.I configVariableBase.h \
    configVariableBool.cxx configVariableBool.I configVariableBool.h \
    configVariableCore.cxx configVariableCore.I configVariableCore.h \
    configVariableDouble.cxx configVariableDouble.I configVariableDouble.h \
    configVariableEnum.cxx configVariableEnum.I configVariableEnum.h \
    configVariableFilename.cxx configVariableFilename.I configVariableFilename.h \
    configVariableInt.cxx configVariableInt.I configVariableInt.h \
    configVariableList.cxx configVariableList.I configVariableList.h \
    configVariableManager.cxx configVariableManager.I configVariableManager.h \
    configVariableSearchPath.cxx configVariableSearchPath.I configVariableSearchPath.h \
    configVariableString.cxx configVariableString.I configVariableString.h \
    globPattern.cxx globPattern.I globPattern.h \
    notify.cxx notify.I notify.h \
    notifyCategory.cxx notifyCategory.I notifyCategory.h \
    notifyCategoryProxy.I notifyCategoryProxy.h \
    notifySeverity.cxx notifySeverity.h \
    prcKeyRegistry.cxx prcKeyRegistry.h \
  
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

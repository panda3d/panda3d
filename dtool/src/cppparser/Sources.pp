#define LOCAL_LIBS dtoolutil dtoolbase
#define YACC_PREFIX cppyy

#begin static_lib_target
  #define TARGET cppParser
  
  #define SOURCES                                                         \
    cppArrayType.cxx cppArrayType.h \
    cppBison.yxx cppBisonDefs.h                                           \
    cppClassTemplateParameter.cxx cppClassTemplateParameter.h             \
    cppCommentBlock.cxx cppCommentBlock.h cppConstType.cxx                \
    cppConstType.h cppDeclaration.cxx cppDeclaration.h cppEnumType.cxx    \
    cppEnumType.h cppExpression.cxx cppExpression.h                       \
    cppExpressionParser.cxx cppExpressionParser.h cppExtensionType.cxx    \
    cppExtensionType.h cppFile.cxx cppFile.h cppFunctionGroup.cxx         \
    cppFunctionGroup.h cppFunctionType.cxx cppFunctionType.h              \
    cppGlobals.cxx cppGlobals.h cppIdentifier.cxx cppIdentifier.h         \
    cppInstance.cxx cppInstance.h cppInstanceIdentifier.cxx               \
    cppInstanceIdentifier.h cppManifest.cxx cppManifest.h                 \
    cppNameComponent.cxx cppNameComponent.h cppNamespace.cxx              \
    cppNamespace.h cppParameterList.cxx cppParameterList.h cppParser.cxx  \
    cppParser.h cppPointerType.cxx cppPointerType.h cppPreprocessor.cxx   \
    cppPreprocessor.h cppReferenceType.cxx cppReferenceType.h             \
    cppScope.cxx cppScope.h cppSimpleType.cxx cppSimpleType.h             \
    cppStructType.cxx cppStructType.h cppTBDType.cxx cppTBDType.h         \
    cppTemplateParameterList.cxx cppTemplateParameterList.h               \
    cppTemplateScope.cxx cppTemplateScope.h cppToken.cxx cppToken.h       \
    cppType.cxx cppType.h cppTypeDeclaration.cxx cppTypeDeclaration.h     \
    cppTypeParser.cxx cppTypeParser.h cppTypeProxy.cxx cppTypeProxy.h     \
    cppTypedef.cxx cppTypedef.h cppUsing.cxx cppUsing.h cppVisibility.cxx \
    cppVisibility.h indent.cxx indent.h

// These are temporary; they need not be installed in the future.  These are
// necessary only when using template stopgap.
  #define INSTALL_HEADERS \
    cppDeclaration.h cppExtensionType.h cppIdentifier.h cppInstance.h \
    cppManifest.h cppPreprocessor.h cppScope.h cppToken.h cppType.h \
    cppVisibility.h cppBisonDefs.h cppParser.h cppInstanceIdentifier.h \
    cppFunctionType.h cppSimpleType.h cppParameterList.h cppTypedef.h \
    cppTypeDeclaration.h \
    cppPointerType.h cppReferenceType.h cppConstType.h cppArrayType.h \
    cppEnumType.h cppStructType.h cppFile.h cppTemplateParameterList.h \
    cppFunctionGroup.h cppNameComponent.h cppTypeProxy.h cppTBDType.h \
    cppExpressionParser.h cppExpression.h cppGlobals.h cppCommentBlock.h

#end static_lib_target

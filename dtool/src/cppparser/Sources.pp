#define DIRECTORY_IF_INTERROGATE yes

#define LOCAL_LIBS dtoolutil dtoolbase
#define YACC_PREFIX cppyy

#begin static_lib_target
  #define TARGET cppParser

  // The composite files are named cppparser_composite?.cxx instead of
  // cppParser_composite?.cxx.
  #define COMBINED_SOURCES cppparser_composite1.cxx  cppparser_composite2.cxx  
  
  #define SOURCES \
     cppArrayType.h cppBison.yxx cppBisonDefs.h  \
     cppClassTemplateParameter.h cppCommentBlock.h cppConstType.h  \
     cppDeclaration.h cppEnumType.h cppExpression.h  \
     cppExpressionParser.h cppExtensionType.h cppFile.h  \
     cppFunctionGroup.h cppFunctionType.h cppGlobals.h  \
     cppIdentifier.h cppInstance.h cppInstanceIdentifier.h  \
     cppManifest.h cppNameComponent.h cppNamespace.h  \
     cppParameterList.h cppParser.h cppPointerType.h  \
     cppPreprocessor.h cppReferenceType.h cppScope.h  \
     cppSimpleType.h cppStructType.h cppTBDType.h  \
     cppTemplateParameterList.h cppTemplateScope.h cppToken.h  \
     cppType.h cppTypeDeclaration.h cppTypeParser.h  \
     cppTypeProxy.h cppTypedef.h cppUsing.h cppVisibility.h  \
     indent.h 

  #define INCLUDED_SOURCES  \
     cppArrayType.cxx cppClassTemplateParameter.cxx  \
     cppCommentBlock.cxx cppConstType.cxx cppDeclaration.cxx  \
     cppEnumType.cxx cppExpression.cxx cppExpressionParser.cxx  \
     cppExtensionType.cxx cppFile.cxx cppFunctionGroup.cxx  \
     cppFunctionType.cxx cppGlobals.cxx cppIdentifier.cxx  \
     cppInstance.cxx cppInstanceIdentifier.cxx cppManifest.cxx  \
     cppNameComponent.cxx cppNamespace.cxx cppParameterList.cxx  \
     cppParser.cxx cppPointerType.cxx cppPreprocessor.cxx  \
     cppReferenceType.cxx cppScope.cxx cppSimpleType.cxx  \
     cppStructType.cxx cppTBDType.cxx  \
     cppTemplateParameterList.cxx cppTemplateScope.cxx  \
     cppToken.cxx cppType.cxx cppTypeDeclaration.cxx  \
     cppTypeParser.cxx cppTypeProxy.cxx cppTypedef.cxx  \
     cppUsing.cxx cppVisibility.cxx indent.cxx 

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

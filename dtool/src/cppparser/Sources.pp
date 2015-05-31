#define BUILD_DIRECTORY $[HAVE_INTERROGATE]

#define LOCAL_LIBS p3dtoolutil p3dtoolbase
#define YACC_PREFIX cppyy

#begin static_lib_target
  #define TARGET p3cppParser

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx  
  
  #define SOURCES \
     cppArrayType.h cppBison.yxx cppBisonDefs.h  \
     cppClassTemplateParameter.h cppCommentBlock.h cppConstType.h  \
     cppDeclaration.h cppEnumType.h cppExpression.h  \
     cppExpressionParser.h cppExtensionType.h cppFile.h  \
     cppFunctionGroup.h cppFunctionType.h cppGlobals.h  \
     cppIdentifier.h cppInstance.h cppInstanceIdentifier.h  \
     cppMakeProperty.h cppMakeSeq.h cppManifest.h \
     cppNameComponent.h cppNamespace.h  \
     cppParameterList.h cppParser.h cppPointerType.h  \
     cppPreprocessor.h cppReferenceType.h cppScope.h  \
     cppSimpleType.h cppStructType.h cppTBDType.h  \
     cppTemplateParameterList.h cppTemplateScope.h cppToken.h  \
     cppType.h cppTypeDeclaration.h cppTypeParser.h  \
     cppTypeProxy.h cppTypedefType.h cppUsing.h cppVisibility.h

  #define INCLUDED_SOURCES  \
     cppArrayType.cxx cppClassTemplateParameter.cxx  \
     cppCommentBlock.cxx cppConstType.cxx cppDeclaration.cxx  \
     cppEnumType.cxx cppExpression.cxx cppExpressionParser.cxx  \
     cppExtensionType.cxx cppFile.cxx cppFunctionGroup.cxx  \
     cppFunctionType.cxx cppGlobals.cxx cppIdentifier.cxx  \
     cppInstance.cxx cppInstanceIdentifier.cxx \
     cppMakeProperty.cxx cppMakeSeq.cxx cppManifest.cxx  \
     cppNameComponent.cxx cppNamespace.cxx cppParameterList.cxx  \
     cppParser.cxx cppPointerType.cxx cppPreprocessor.cxx  \
     cppReferenceType.cxx cppScope.cxx cppSimpleType.cxx  \
     cppStructType.cxx cppTBDType.cxx  \
     cppTemplateParameterList.cxx cppTemplateScope.cxx  \
     cppToken.cxx cppType.cxx cppTypeDeclaration.cxx  \
     cppTypeParser.cxx cppTypeProxy.cxx cppTypedefType.cxx  \
     cppUsing.cxx cppVisibility.cxx

#end static_lib_target

set(P3CPPPARSER_HEADERS
  cppArrayType.h cppBison.yxx cppBisonDefs.h
  cppClassTemplateParameter.h cppCommentBlock.h
  cppClosureType.h cppConstType.h
  cppDeclaration.h cppEnumType.h cppExpression.h
  cppExpressionParser.h cppExtensionType.h cppFile.h
  cppFunctionGroup.h cppFunctionType.h cppGlobals.h
  cppIdentifier.h cppInstance.h cppInstanceIdentifier.h
  cppMakeProperty.h cppMakeSeq.h cppManifest.h
  cppNameComponent.h cppNamespace.h
  cppParameterList.h cppParser.h cppPointerType.h
  cppPreprocessor.h cppReferenceType.h cppScope.h
  cppSimpleType.h cppStructType.h cppTBDType.h
  cppTemplateParameterList.h cppTemplateScope.h cppToken.h
  cppType.h cppTypeDeclaration.h cppTypeParser.h
  cppTypeProxy.h cppTypedefType.h cppUsing.h cppVisibility.h
)

set(P3CPPPARSER_SOURCES
  cppArrayType.cxx ${CMAKE_CURRENT_BINARY_DIR}/cppBison.cxx
  cppClassTemplateParameter.cxx
  cppCommentBlock.cxx cppClosureType.cxx cppConstType.cxx cppDeclaration.cxx
  cppEnumType.cxx cppExpression.cxx cppExpressionParser.cxx
  cppExtensionType.cxx cppFile.cxx cppFunctionGroup.cxx
  cppFunctionType.cxx cppGlobals.cxx cppIdentifier.cxx
  cppInstance.cxx cppInstanceIdentifier.cxx
  cppMakeProperty.cxx cppMakeSeq.cxx cppManifest.cxx
  cppNameComponent.cxx cppNamespace.cxx cppParameterList.cxx
  cppParser.cxx cppPointerType.cxx cppPreprocessor.cxx
  cppReferenceType.cxx cppScope.cxx cppSimpleType.cxx
  cppStructType.cxx cppTBDType.cxx
  cppTemplateParameterList.cxx cppTemplateScope.cxx
  cppToken.cxx cppType.cxx cppTypeDeclaration.cxx
  cppTypeParser.cxx cppTypeProxy.cxx cppTypedefType.cxx
  cppUsing.cxx cppVisibility.cxx
)

add_bison_target(cppBison.cxx cppBison.yxx DEFINES cppBison.h PREFIX cppyy)

composite_sources(p3cppParser P3CPPPARSER_SOURCES)
add_library(p3cppParser STATIC ${P3CPPPARSER_HEADERS} ${P3CPPPARSER_SOURCES})
target_link_libraries(p3cppParser p3dtool)

install(TARGETS p3cppParser EXPORT Core COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_LIBDIR})

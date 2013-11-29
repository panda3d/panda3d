include_directories(${CMAKE_CURRENT_LIST_DIR}/../dtoolutil)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../dtoolbase)

set(P3CPPPARSER_HEADER_FILES
	cppArrayType.h cppBison.yxx cppBisonDefs.h
	cppClassTemplateParameter.h cppCommentBlock.h cppConstType.h
	cppDeclaration.h cppEnumType.h cppExpression.h
	cppExpressionParser.h cppExtensionType.h cppFile.h
	cppFunctionGroup.h cppFunctionType.h cppGlobals.h
	cppIdentifier.h cppInstance.h cppInstanceIdentifier.h
	cppMakeSeq.h cppManifest.h
	cppNameComponent.h cppNamespace.h
	cppParameterList.h cppParser.h cppPointerType.h
	cppPreprocessor.h cppReferenceType.h cppScope.h
	cppSimpleType.h cppStructType.h cppTBDType.h
	cppTemplateParameterList.h cppTemplateScope.h cppToken.h
	cppType.h cppTypeDeclaration.h cppTypeParser.h
	cppTypeProxy.h cppTypedef.h cppUsing.h cppVisibility.h)

set(P3CPPPARSER_SOURCE_FILES
	cppArrayType.cxx cppBison.cxx cppClassTemplateParameter.cxx
	cppCommentBlock.cxx cppConstType.cxx cppDeclaration.cxx
	cppEnumType.cxx cppExpression.cxx cppExpressionParser.cxx
	cppExtensionType.cxx cppFile.cxx cppFunctionGroup.cxx
	cppFunctionType.cxx cppGlobals.cxx cppIdentifier.cxx
	cppInstance.cxx cppInstanceIdentifier.cxx
	cppMakeSeq.cxx cppManifest.cxx
	cppNameComponent.cxx cppNamespace.cxx cppParameterList.cxx
	cppParser.cxx cppPointerType.cxx cppPreprocessor.cxx
	cppReferenceType.cxx cppScope.cxx cppSimpleType.cxx
	cppStructType.cxx cppTBDType.cxx
	cppTemplateParameterList.cxx cppTemplateScope.cxx
	cppToken.cxx cppType.cxx cppTypeDeclaration.cxx
	cppTypeParser.cxx cppTypeProxy.cxx cppTypedef.cxx
	cppUsing.cxx cppVisibility.cxx)

foreach(FILENAME IN LISTS P3CPPPARSER_HEADER_FILES)
	set(P3CPPPARSER_HEADERS ${P3CPPPARSER_HEADERS} ${CMAKE_CURRENT_LIST_DIR}/${FILENAME})
endforeach()
foreach(FILENAME IN LISTS P3CPPPARSER_SOURCE_FILES)
	set(P3CPPPARSER_SOURCES ${P3CPPPARSER_SOURCES} ${CMAKE_CURRENT_LIST_DIR}/${FILENAME})
endforeach()

if(HAVE_INTERROGATE)
	find_package(BISON)
	bison_target(cppParser
		${CMAKE_CURRENT_LIST_DIR}/cppBison.yxx
		${CMAKE_CURRENT_LIST_DIR}/cppBison.cxx
		COMPILE_FLAGS --defines=${CMAKE_CURRENT_LIST_DIR}/cppBison.h)

	add_library(p3cppParser STATIC
		${P3CPPPARSER_HEADERS}
		${P3CPPPARSER_SOURCES})
	add_dependencies(p3cppParser cppParser)
endif()

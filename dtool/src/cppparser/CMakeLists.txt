include_directories(../dtoolutil)
include_directories(../dtoolbase)

set(P3CPPPARSER_HEADERS
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
	cppTypeProxy.h cppTypedef.h cppUsing.h cppVisibility.h
)

set(P3CPPPARSER_SOURCES
	cppArrayType.cxx ${CMAKE_CURRENT_BINARY_DIR}/cppBison.cxx
	cppClassTemplateParameter.cxx
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
	cppUsing.cxx cppVisibility.cxx
)

# Bison is required for CPPParser
# Check for it here, so that cppparser can be compiled individually
if(NOT HAVE_BISON)
	find_package(BISON REQUIRED QUIET)

	if(NOT BISON_FOUND)
		message(FATAL_ERROR "CPPParser requires Bison.")
	endif()
endif()

# TODO: we should move this to a global macro somewhere.
if(BISON_FOUND)
  add_custom_command(
    OUTPUT cppBison.h cppBison.cxx
    COMMAND ${BISON_EXECUTABLE}
      --defines=cppBison.h -o cppBison.cxx -p cppyy
      "${CMAKE_CURRENT_SOURCE_DIR}/cppBison.yxx"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cppBison.yxx"
  )
else()
  add_custom_command(
    OUTPUT cppBison.h cppBison.cxx
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/cppBison.h.prebuilt" cppBison.h
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/cppBison.cxx.prebuilt" cppBison.cxx
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cppBison.h.prebuilt" "${CMAKE_CURRENT_SOURCE_DIR}/cppBison.cxx.prebuilt"
  )
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(p3cppParser STATIC
	${P3CPPPARSER_HEADERS}
	${P3CPPPARSER_SOURCES}
)
add_dependencies(p3cppParser cppParser)

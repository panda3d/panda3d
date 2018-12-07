add_bison_target(dcParser.cxx dcParser.yxx DEFINES dcParser.h PREFIX dcyy)
add_flex_target(dcLexer.cxx dcLexer.lxx CASE_INSENSITIVE PREFIX dcyy)

set(P3DCPARSER_HEADERS
     dcAtomicField.h dcAtomicField.I dcClass.h dcClass.I
     dcDeclaration.h
     dcField.h dcField.I
     dcFile.h dcFile.I
     dcKeyword.h dcKeywordList.h
     dcLexer.lxx
     dcLexerDefs.h dcMolecularField.h dcParser.yxx dcParserDefs.h
     dcSubatomicType.h
     dcPackData.h dcPackData.I
     dcPacker.h dcPacker.I
     dcPackerCatalog.h dcPackerCatalog.I
     dcPackerInterface.h dcPackerInterface.I
     dcParameter.h dcClassParameter.h dcArrayParameter.h
     dcSimpleParameter.h dcSwitchParameter.h
     dcNumericRange.h dcNumericRange.I
     dcSwitch.h
     dcTypedef.h
     dcPython.h
     dcbase.h dcindent.h
     dcmsgtypes.h
     hashGenerator.h
     primeNumberGenerator.h)

set(P3DCPARSER_SOURCES
     dcAtomicField.cxx dcClass.cxx
     dcDeclaration.cxx
     dcField.cxx dcFile.cxx
     dcKeyword.cxx dcKeywordList.cxx
     dcMolecularField.cxx dcSubatomicType.cxx
     dcPackData.cxx
     dcPacker.cxx
     dcPackerCatalog.cxx
     dcPackerInterface.cxx
     dcParameter.cxx dcClassParameter.cxx dcArrayParameter.cxx
     dcSimpleParameter.cxx dcSwitchParameter.cxx
     dcSwitch.cxx
     dcTypedef.cxx
     dcindent.cxx
     hashGenerator.cxx primeNumberGenerator.cxx)

# These cannot be interrogated, and are excluded from the composites.
set(P3DCPARSER_PARSER_SOURCES
    dcParser.cxx
    dcLexer.cxx)

composite_sources(p3dcparser P3DCPARSER_SOURCES)
add_component_library(p3dcparser NOINIT SYMBOL BUILDING_DIRECT_DCPARSER
  ${P3DCPARSER_HEADERS} ${P3DCPARSER_SOURCES} ${P3DCPARSER_PARSER_SOURCES})
target_compile_definitions(p3dcparser PUBLIC WITHIN_PANDA)
target_link_libraries(p3dcparser p3directbase panda PKG::PYTHON)
target_interrogate(p3dcparser ${P3DCPARSER_HEADERS} ${P3DCPARSER_SOURCES})

if(NOT BUILD_METALIBS)
  install(TARGETS p3dcparser
    EXPORT Direct COMPONENT Direct
    DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE COMPONENT DirectDevel)
endif()
install(FILES ${P3DCPARSER_HEADERS} COMPONENT DirectDevel DESTINATION include/panda3d)

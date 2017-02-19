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
     dcbase.h dcindent.h hashGenerator.h
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
add_library(p3dcparser ${P3DCPARSER_HEADERS} ${P3DCPARSER_SOURCES}
  ${P3DCPARSER_PARSER_SOURCES})
set_target_properties(p3dcparser PROPERTIES COMPILE_DEFINITIONS WITHIN_PANDA)
target_link_libraries(p3dcparser p3directbase p3express p3pstatclient p3prc)
target_interrogate(p3dcparser ${P3DCPARSER_HEADERS} ${P3DCPARSER_SOURCES})

install(TARGETS p3dcparser DESTINATION lib)
install(FILES ${P3DCPARSER_HEADERS} DESTINATION include/panda3d)

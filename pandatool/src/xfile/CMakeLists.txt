set(P3XFILE_HEADERS
  config_xfile.h
  standard_templates.h
  windowsGuid.h windowsGuid.I
  xFileArrayDef.h xFileArrayDef.I
  xFileDataDef.h xFileDataDef.I
  xFileDataNode.h xFileDataNode.I
  xFileDataNodeReference.h xFileDataNodeReference.I
  xFileDataNodeTemplate.h xFileDataNodeTemplate.I
  xFileDataObjectArray.h xFileDataObjectArray.I
  xFileDataObjectDouble.h xFileDataObjectDouble.I
  xFileDataObject.h xFileDataObject.I
  xFileDataObjectInteger.h xFileDataObjectInteger.I
  xFileDataObjectString.h xFileDataObjectString.I
  xFile.h xFile.I
  xFileNode.h xFileNode.I
  xFileParseData.h xFileParseData.I
  xFileTemplate.h xFileTemplate.I
  xLexerDefs.h
  xParserDefs.h
)

set(P3XFILE_SOURCES
  config_xfile.cxx
  standard_templates.cxx
  windowsGuid.cxx
  xFileArrayDef.cxx
  xFile.cxx
  xFileDataDef.cxx
  xFileDataNode.cxx
  xFileDataNodeReference.cxx
  xFileDataNodeTemplate.cxx
  xFileDataObjectArray.cxx
  xFileDataObject.cxx
  xFileDataObjectDouble.cxx
  xFileDataObjectInteger.cxx
  xFileDataObjectString.cxx
  xFileNode.cxx
  xFileParseData.cxx
  xFileTemplate.cxx
)

set(P3XFILE_PARSER_SOURCES
  xParser.cxx
  xLexer.cxx
)

add_bison_target(xParser.cxx xParser.yxx DEFINES xParser.h PREFIX xyy)
add_flex_target(xLexer.cxx xLexer.lxx CASE_INSENSITIVE PREFIX xyy)

composite_sources(p3xfile P3XFILE_SOURCES)
add_library(p3xfile STATIC ${P3XFILE_HEADERS} ${P3XFILE_SOURCES} ${P3XFILE_PARSER_SOURCES})
target_link_libraries(p3xfile p3pandatoolbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

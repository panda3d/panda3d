set(P3VRML_HEADERS
  parse_vrml.h
  standard_nodes.h
  vrmlLexerDefs.h
  vrmlNode.h
  vrmlNodeType.h
  vrmlParserDefs.h
)

set(P3VRML_SOURCES
  parse_vrml.cxx
  standard_nodes.cxx
  vrmlNode.cxx
  vrmlNodeType.cxx
)

set(P3VRML_PARSER_SOURCES
  vrmlParser.cxx
  vrmlLexer.cxx
)

add_bison_target(vrmlParser.cxx vrmlParser.yxx DEFINES vrmlParser.h PREFIX vrmlyy)
add_flex_target(vrmlLexer.cxx vrmlLexer.lxx CASE_INSENSITIVE PREFIX vrmlyy)

composite_sources(p3vrml P3VRML_SOURCES)
add_library(p3vrml STATIC ${P3VRML_HEADERS} ${P3VRML_SOURCES} ${P3VRML_PARSER_SOURCES})
target_link_libraries(p3vrml p3pandatoolbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.

#define YACC_PREFIX xyy
#define LFLAGS -i

#begin ss_lib_target
  #define TARGET xfile
  #define LOCAL_LIBS pandatoolbase
  #define OTHER_LIBS \
    mathutil:c linmath:c panda:m \
    dtoolbase:c dtool:m

  #define USE_PACKAGES zlib
    
  #define SOURCES \
     config_xfile.h config_xfile.cxx \
     standard_templates.cxx standard_templates.h \
     windowsGuid.cxx windowsGuid.h \
     xLexer.lxx xLexerDefs.h \
     xParser.yxx xParserDefs.h \
     xFile.cxx xFile.I xFile.h \
     xFileArrayDef.cxx xFileArrayDef.I xFileArrayDef.h \
     xFileDataDef.cxx xFileDataDef.I xFileDataDef.h \
     xFileDataObject.cxx xFileDataObject.I xFileDataObject.h \
     xFileDataObjectArray.cxx xFileDataObjectArray.I xFileDataObjectArray.h \
     xFileDataObjectDouble.cxx xFileDataObjectDouble.I xFileDataObjectDouble.h \
     xFileDataObjectInteger.cxx xFileDataObjectInteger.I xFileDataObjectInteger.h \
     xFileDataObjectString.cxx xFileDataObjectString.I xFileDataObjectString.h \
     xFileDataNode.cxx xFileDataNode.I xFileDataNode.h \
     xFileDataNodeReference.cxx xFileDataNodeReference.I xFileDataNodeReference.h \
     xFileDataNodeTemplate.cxx xFileDataNodeTemplate.I xFileDataNodeTemplate.h \
     xFileNode.cxx xFileNode.I xFileNode.h \
     xFileParseData.cxx xFileParseData.I xFileParseData.h \
     xFileTemplate.cxx xFileTemplate.I xFileTemplate.h

#end ss_lib_target

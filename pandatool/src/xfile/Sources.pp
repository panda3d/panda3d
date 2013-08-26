#define YACC_PREFIX xyy
#define FLEXFLAGS -i

#define USE_PACKAGES zlib

#begin ss_lib_target
  #define TARGET p3xfile
  #define LOCAL_LIBS p3pandatoolbase
  #define OTHER_LIBS \
    p3event:c p3putil:c p3pipeline:c p3mathutil:c p3linmath:c panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m

  #define USE_PACKAGES zlib

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx   
    
  #define SOURCES \
     config_xfile.h \
     standard_templates.h \
     windowsGuid.h \
     xFile.I xFile.h \
     xFileArrayDef.I xFileArrayDef.h \
     xFileDataDef.I xFileDataDef.h \
     xFileDataNode.I xFileDataNode.h \
     xFileDataNodeReference.I xFileDataNodeReference.h \
     xFileDataNodeTemplate.I xFileDataNodeTemplate.h \
     xFileDataObject.I xFileDataObject.h \
     xFileDataObjectArray.I xFileDataObjectArray.h \
     xFileDataObjectDouble.I xFileDataObjectDouble.h \
     xFileDataObjectInteger.I xFileDataObjectInteger.h \
     xFileDataObjectString.I xFileDataObjectString.h \
     xFileNode.I xFileNode.h \
     xFileParseData.I xFileParseData.h \
     xFileTemplate.I xFileTemplate.h \
     xLexer.lxx xLexerDefs.h \
     xParser.yxx xParserDefs.h

  #define INCLUDED_SOURCES \
     config_xfile.cxx \
     standard_templates.cxx \
     windowsGuid.cxx\
     xFile.cxx \
     xFileArrayDef.cxx \
     xFileDataDef.cxx \
     xFileDataNode.cxx \
     xFileDataNodeReference.cxx \
     xFileDataNodeTemplate.cxx \
     xFileDataObject.cxx \
     xFileDataObjectArray.cxx \
     xFileDataObjectDouble.cxx \
     xFileDataObjectInteger.cxx \
     xFileDataObjectString.cxx \
     xFileNode.cxx \
     xFileParseData.cxx \
     xFileTemplate.cxx


#end ss_lib_target

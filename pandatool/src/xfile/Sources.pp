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
     xFileDataObjectTemplate.cxx xFileDataObjectTemplate.I xFileDataObjectTemplate.h \
     xFileNode.cxx xFileNode.I xFileNode.h \
     xFileTemplate.cxx xFileTemplate.I xFileTemplate.h

#end ss_lib_target

#begin test_bin_target
  #define TARGET x-trans
  #define LOCAL_LIBS \
    progbase xfile
  #define OTHER_LIBS \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    xFileTrans.cxx xFileTrans.h

#end test_bin_target

#define OTHER_LIBS \
    express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS \
    directbase
#define YACC_PREFIX dcyy
#define C++FLAGS -DWITHIN_PANDA
#define UNIX_SYS_LIBS m
#define USE_PACKAGES python

#begin lib_target
  #define TARGET dcparser

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx

  #define SOURCES \
     dcAtomicField.h dcClass.h dcField.h dcFile.h dcLexer.lxx  \
     dcLexerDefs.h dcMolecularField.h dcParser.yxx dcParserDefs.h  \
     dcSubatomicType.h \
     dcPackData.h dcPackData.I \
     dcPacker.h dcPacker.I \
     dcPackerInterface.h \
     dcParameter.h dcArrayParameter.h dcSimpleParameter.h \
     dcTypedef.h \
     dcbase.h dcindent.h hashGenerator.h  \
     primeNumberGenerator.h  

  #define INCLUDED_SOURCES \
     dcAtomicField.cxx dcClass.cxx dcField.cxx dcFile.cxx \
     dcMolecularField.cxx dcSubatomicType.cxx \
     dcPackData.cxx \
     dcPacker.cxx \
     dcPackerInterface.cxx \
     dcParameter.cxx dcArrayParameter.cxx dcSimpleParameter.cxx \
     dcTypedef.cxx \
     dcindent.cxx  \
     hashGenerator.cxx primeNumberGenerator.cxx 

  #define IGATESCAN all
#end lib_target

#define OTHER_LIBS \
    express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS \
    directbase
#define YACC_PREFIX dcyy
#define C++FLAGS -DWITHIN_PANDA
#define UNIX_SYS_LIBS m

#begin lib_target
  #define TARGET dcparser

  #define SOURCES \
    dcAtomicField.cxx dcAtomicField.h dcClass.cxx dcClass.h \
    dcField.cxx dcField.h dcFile.cxx dcFile.h \
    dcLexer.lxx dcLexerDefs.h \
    dcMolecularField.cxx dcMolecularField.h dcParser.yxx \
    dcParserDefs.h dcSubatomicType.cxx dcSubatomicType.h dcbase.h \
    dcindent.cxx dcindent.h \
    hashGenerator.cxx hashGenerator.h \
    primeNumberGenerator.h primeNumberGenerator.cxx

  #define IGATESCAN all
#end lib_target

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define YACC_PREFIX dcyy

#begin lib_target
  #define TARGET dcparse

  #define SOURCES \
    dcAtomicField.cxx dcAtomicField.h dcClass.cxx dcClass.h \
    dcField.cxx dcField.h dcFile.cxx dcFile.h dcLexer.lxx dcLexerDefs.h \
    dcMolecularField.cxx dcMolecularField.h dcParser.yxx \
    dcParserDefs.h dcSubatomicType.cxx dcSubatomicType.h dcbase.h \
    indent.cxx indent.h

  #define EXTRA_DIST test.dc dc.dsp dc.dsw

  #define IGATESCAN all
#end lib_target

#begin test_bin_target
  #define TARGET dcparse
  #define LOCAL_LIBS dcparse
  #define OTHER_LIBS $[OTHER_LIBS] pystub

  #define SOURCES \
    dcparse.cxx

  #define EXTRA_DIST test.dc dc.dsp dc.dsw
#end test_bin_target


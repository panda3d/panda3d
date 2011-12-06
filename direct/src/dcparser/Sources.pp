#define OTHER_LIBS \
    p3express:c pandaexpress:m \
    p3pstatclient:c p3pipeline:c panda:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c p3pandabase:c \
    p3downloader:c $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3linmath:c p3putil:c

#define LOCAL_LIBS \
    p3directbase
#define YACC_PREFIX dcyy
#define C++FLAGS -DWITHIN_PANDA
#define UNIX_SYS_LIBS m
#define USE_PACKAGES python

#begin lib_target
  #define TARGET p3dcparser

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx

  #define SOURCES \
     dcAtomicField.h dcAtomicField.I dcClass.h dcClass.I \
     dcDeclaration.h \
     dcField.h dcField.I \
     dcFile.h dcFile.I \
     dcKeyword.h dcKeywordList.h \
     dcLexer.lxx  \
     dcLexerDefs.h dcMolecularField.h dcParser.yxx dcParserDefs.h  \
     dcSubatomicType.h \
     dcPackData.h dcPackData.I \
     dcPacker.h dcPacker.I \
     dcPackerCatalog.h dcPackerCatalog.I \
     dcPackerInterface.h dcPackerInterface.I \
     dcParameter.h dcClassParameter.h dcArrayParameter.h \
     dcSimpleParameter.h dcSwitchParameter.h \
     dcNumericRange.h dcNumericRange.I \
     dcSwitch.h \
     dcTypedef.h \
     dcPython.h \
     dcbase.h dcindent.h hashGenerator.h  \
     primeNumberGenerator.h  

  #define INCLUDED_SOURCES \
     dcAtomicField.cxx dcClass.cxx \
     dcDeclaration.cxx \
     dcField.cxx dcFile.cxx \
     dcKeyword.cxx dcKeywordList.cxx \
     dcMolecularField.cxx dcSubatomicType.cxx \
     dcPackData.cxx \
     dcPacker.cxx \
     dcPackerCatalog.cxx \
     dcPackerInterface.cxx \
     dcParameter.cxx dcClassParameter.cxx dcArrayParameter.cxx \
     dcSimpleParameter.cxx dcSwitchParameter.cxx \
     dcSwitch.cxx \
     dcTypedef.cxx \
     dcindent.cxx  \
     hashGenerator.cxx primeNumberGenerator.cxx 

  //  #define SOURCES $[SOURCES] $[INCLUDED_SOURCES]
  //  #define COMBINED_SOURCES

  #define IGATESCAN all
#end lib_target

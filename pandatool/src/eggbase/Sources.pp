#begin ss_lib_target
  #define TARGET eggbase
  #define LOCAL_LIBS \
    progbase
  #define OTHER_LIBS \
    egg:c linmath:c panda:m

  #define SOURCES \
    eggBase.cxx eggBase.h \
    eggConverter.cxx eggConverter.h \
    eggFilter.cxx eggFilter.h \
    eggMultiBase.cxx eggMultiBase.h \
    eggMultiFilter.cxx eggMultiFilter.h \
    eggReader.cxx eggReader.h \
    eggToSomething.cxx eggToSomething.h \
    eggWriter.cxx eggWriter.h \
    somethingToEgg.cxx somethingToEgg.h

  #define INSTALL_HEADERS \
    eggBase.h eggConverter.h eggFilter.h \
    eggMultiBase.h eggMultiFilter.h \
    eggReader.h eggToSomething.h eggWriter.h somethingToEgg.h

#end ss_lib_target


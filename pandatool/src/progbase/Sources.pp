#begin ss_lib_target
  #define TARGET progbase
  #define LOCAL_LIBS \
    converter pandatoolbase
  #define OTHER_LIBS \
    linmath:c putil:c express:c panda:m pystub dtool

  #define SOURCES \
    programBase.I programBase.cxx programBase.h \
    withOutputFile.cxx withOutputFile.h \
    wordWrapStream.cxx \
    wordWrapStream.h wordWrapStreamBuf.I wordWrapStreamBuf.cxx \
    wordWrapStreamBuf.h

  #define INSTALL_HEADERS \
    programBase.I programBase.h \
    withOutputFile.h \
    wordWrapStream.h wordWrapStreamBuf.I \
    wordWrapStreamBuf.h

#end ss_lib_target

#begin test_bin_target
  #define TARGET test_prog
  #define LOCAL_LIBS \
    progbase

  #define SOURCES \
    test_prog.cxx

#end test_bin_target


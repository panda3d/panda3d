#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                  dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS dgraph putil express pandabase

#begin lib_target
  #define TARGET recorder
 
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx   
  
  #define SOURCES \
    config_recorder.h \
    mouseRecorder.h \
    recorderBase.h recorderBase.I \
    recorderController.h recorderController.I \
    recorderFrame.h recorderFrame.I \
    recorderHeader.h recorderHeader.I \
    recorderTable.h recorderTable.I \
    socketStreamRecorder.h socketStreamRecorder.I
    
 #define INCLUDED_SOURCES \
    config_recorder.cxx \
    mouseRecorder.cxx \
    recorderBase.cxx recorderController.cxx \
    recorderFrame.cxx recorderHeader.cxx recorderTable.cxx \
    socketStreamRecorder.cxx

  #define INSTALL_HEADERS \
    mouseRecorder.h \
    recorderBase.h recorderBase.I \
    recorderController.h recorderController.I \
    recorderFrame.h recorderFrame.I \
    recorderHeader.h recorderHeader.I \
    recorderTable.h recorderTable.I \
    socketStreamRecorder.h socketStreamRecorder.I

  #define IGATESCAN all

#end lib_target

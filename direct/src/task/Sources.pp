#begin lib_target
  #define TARGET task
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    panda

  #define SOURCES \
    config_task.cxx config_task.h \
    cTask.h cTask.I cTask.cxx

  #define INSTALL_HEADERS \
    config_task.h \
    cTask.h cTask.I

  #define IGATESCAN all
#end lib_target

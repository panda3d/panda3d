#define DIRECTORY_IF_GTKMM yes
#define USE_GTKMM yes

#begin lib_target
  #define TARGET gtkbase
  #define LOCAL_LIBS \
    progbase

  #define SOURCES \
    basicGtkDialog.cxx basicGtkDialog.h basicGtkWindow.cxx \
    basicGtkWindow.h gtkBase.cxx gtkBase.h request_initial_size.cxx \
    request_initial_size.h

  #define INSTALL_HEADERS \
    basicGtkDialog.h basicGtkWindow.h gtkBase.h request_initial_size.h

#end lib_target


#define LOCAL_LIBS dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET interrogatedb
  
  #define SOURCES \
    config_interrogatedb.cxx config_interrogatedb.h indexRemapper.cxx	\
    indexRemapper.h interrogateComponent.I interrogateComponent.cxx	\
    interrogateComponent.h interrogateDatabase.I			\
    interrogateDatabase.cxx interrogateDatabase.h interrogateElement.I	\
    interrogateElement.cxx interrogateElement.h interrogateFunction.I	\
    interrogateFunction.cxx interrogateFunction.h			\
    interrogateFunctionWrapper.I interrogateFunctionWrapper.cxx		\
    interrogateFunctionWrapper.h interrogateManifest.I			\
    interrogateManifest.cxx interrogateManifest.h interrogateType.I	\
    interrogateType.cxx interrogateType.h interrogate_datafile.I		\
    interrogate_datafile.cxx interrogate_datafile.h			\
    interrogate_interface.cxx interrogate_interface.h			\
    interrogate_request.cxx interrogate_request.h vector_int.cxx		\
    vector_int.h

  #define INSTALL_HEADERS \
    interrogate_interface.h interrogate_request.h \
    vector_int.h config_interrogatedb.h

// These are temporary; they need not be installed in the future.  These are
// necessary only when using template stopgap.
  #define INSTALL_HEADERS \
    $[INSTALL_HEADERS] \
    interrogateComponent.h interrogateComponent.I \
    interrogateType.h interrogateType.I \
    interrogateFunction.h interrogateFunction.I \
    interrogateFunctionWrapper.h interrogateFunctionWrapper.I \
    interrogateManifest.h interrogateManifest.I \
    interrogateElement.h interrogateElement.I \
    interrogateDatabase.h interrogateDatabase.I \
    indexRemapper.h

#end lib_target

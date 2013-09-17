#define LOCAL_LIBS p3dconfig p3dtoolutil p3dtoolbase

#begin lib_target
  #define TARGET p3interrogatedb
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx
  
  #define SOURCES \
    config_interrogatedb.h indexRemapper.h interrogateComponent.I  \
    interrogateComponent.h interrogateDatabase.I  \
    interrogateDatabase.h interrogateElement.I  \
    interrogateElement.h interrogateFunction.I  \
    interrogateFunction.h interrogateFunctionWrapper.I  \
    interrogateFunctionWrapper.h \
    interrogateMakeSeq.I interrogateMakeSeq.h \
    interrogateManifest.I interrogateManifest.h \
    interrogateType.I interrogateType.h  \
    interrogate_datafile.I interrogate_datafile.h  \
    interrogate_interface.h interrogate_request.h \
    extension.h py_panda.h \
    vector_int.h

 #define INCLUDED_SOURCES  \
    config_interrogatedb.cxx \
    dtool_super_base.cxx \
    indexRemapper.cxx  \
    interrogateComponent.cxx interrogateDatabase.cxx  \
    interrogateElement.cxx interrogateFunction.cxx  \
    interrogateFunctionWrapper.cxx \
    interrogateMakeSeq.cxx  \
    interrogateManifest.cxx  \
    interrogateType.cxx interrogate_datafile.cxx  \
    interrogate_interface.cxx interrogate_request.cxx  \
    py_panda.cxx \
    vector_int.cxx 

  #define INSTALL_HEADERS \
    interrogate_interface.h interrogate_request.h vector_int.h \
    config_interrogatedb.h \
    extension.h py_panda.h \
    vector_int.h

#end lib_target

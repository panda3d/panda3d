#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET dgraph
  #define LOCAL_LIBS \
    pstatclient sgraph graph putil sgattrib mathutil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx  
  
  #define SOURCES \
     buttonEventDataTransition.I buttonEventDataTransition.h  \
     config_dgraph.h dataGraphTraversal.h dataGraphTraverser.I  \
     dataGraphTraverser.h dataNode.h dataRelation.I  \
     dataRelation.N dataRelation.h describe_data_verbose.h  \
     doubleDataTransition.I doubleDataTransition.h  \
     intDataTransition.I intDataTransition.h \
     matrixDataTransition.I matrixDataTransition.h \
     numericDataTransition.I numericDataTransition.h \
     vec3DataTransition.I vec3DataTransition.h \
     vec4DataTransition.I vec4DataTransition.h  \
     vectorDataTransition.I vectorDataTransition.h
    
 #define INCLUDED_SOURCES \
     buttonEventDataTransition.cxx  \
     config_dgraph.cxx dataGraphTraversal.cxx  \
     dataGraphTraverser.cxx dataNode.cxx dataRelation.cxx  \
     describe_data_verbose.cxx \
     doubleDataTransition.cxx  \
     intDataTransition.cxx  \
     matrixDataTransition.cxx  \
     vec3DataTransition.cxx  \
     vec4DataTransition.cxx  

  #define INSTALL_HEADERS \
    buttonEventDataTransition.I buttonEventDataTransition.h \
    dataGraphTraversal.h \
    dataNode.h dataRelation.I dataRelation.h \
    describe_data_verbose.h \
    doubleDataTransition.I doubleDataTransition.h \
    intDataTransition.I intDataTransition.h \
    matrixDataTransition.I matrixDataTransition.h \
    numericDataTransition.I numericDataTransition.h \
    vec3DataTransition.I vec3DataTransition.h \
    vec4DataTransition.I vec4DataTransition.h \
    vectorDataTransition.I vectorDataTransition.h

  #define IGATESCAN \
    dataNode.cxx dataNode.h dataRelation.cxx dataRelation.h \
    dataGraphTraversal.cxx dataGraphTraversal.h

#end lib_target

#begin test_bin_target
  #define TARGET test_dgraph
  #define LOCAL_LIBS \
    dgraph

  #define SOURCES \
    test_dgraph.cxx

#end test_bin_target


#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET dgraph
  #define LOCAL_LIBS \
    pstatclient sgraph graph putil sgattrib mathutil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx  
  
  #define SOURCES \
     buttonEventDataAttribute.I buttonEventDataAttribute.h  \
     buttonEventDataTransition.I buttonEventDataTransition.h  \
     config_dgraph.h dataGraphTraversal.h dataGraphTraverser.I  \
     dataGraphTraverser.h dataNode.h dataRelation.I  \
     dataRelation.N dataRelation.h describe_data_verbose.h  \
     doubleDataAttribute.I doubleDataAttribute.h  \
     doubleDataTransition.I doubleDataTransition.h  \
     doublePtrDataAttribute.I doublePtrDataAttribute.h  \
     doublePtrDataTransition.I doublePtrDataTransition.h  \
     intDataAttribute.I intDataAttribute.h intDataTransition.I  \
     intDataTransition.h matrixDataAttribute.I  \
     matrixDataAttribute.h matrixDataTransition.I  \
     matrixDataTransition.h numericDataAttribute.I  \
     numericDataAttribute.h numericDataTransition.I  \
     numericDataTransition.h pointerDataAttribute.I  \
     pointerDataAttribute.h pointerDataTransition.I  \
     pointerDataTransition.h vec3DataAttribute.I  \
     vec3DataAttribute.h vec3DataTransition.I  \
     vec3DataTransition.h vec4DataAttribute.I vec4DataAttribute.h  \
     vec4DataTransition.I vec4DataTransition.h  \
     vectorDataAttribute.I vectorDataAttribute.h  \
     vectorDataTransition.I vectorDataTransition.h
    
 #define INCLUDED_SOURCES \
     buttonEventDataAttribute.cxx buttonEventDataTransition.cxx  \
     config_dgraph.cxx dataGraphTraversal.cxx  \
     dataGraphTraverser.cxx dataNode.cxx dataRelation.cxx  \
     describe_data_verbose.cxx doubleDataAttribute.cxx  \
     doubleDataTransition.cxx doublePtrDataAttribute.cxx  \
     doublePtrDataTransition.cxx intDataAttribute.cxx  \
     intDataTransition.cxx matrixDataAttribute.cxx  \
     matrixDataTransition.cxx vec3DataAttribute.cxx  \
     vec3DataTransition.cxx vec4DataAttribute.cxx  \
     vec4DataTransition.cxx  

  #define INSTALL_HEADERS \
    buttonEventDataAttribute.I buttonEventDataAttribute.h \
    buttonEventDataTransition.I buttonEventDataTransition.h \
    dataGraphTraversal.h \
    dataNode.h dataRelation.I dataRelation.h \
    describe_data_verbose.h doubleDataAttribute.I doubleDataAttribute.h \
    doubleDataTransition.I doubleDataTransition.h \
    doublePtrDataAttribute.I doublePtrDataAttribute.h \
    doublePtrDataTransition.I doublePtrDataTransition.h \
    intDataAttribute.I intDataAttribute.h intDataTransition.I \
    intDataTransition.h matrixDataAttribute.I matrixDataAttribute.h \
    matrixDataTransition.I matrixDataTransition.h \
    numericDataAttribute.I numericDataAttribute.h \
    numericDataTransition.I numericDataTransition.h \
    pointerDataAttribute.I pointerDataAttribute.h \
    pointerDataTransition.I pointerDataTransition.h vec3DataAttribute.I \
    vec3DataAttribute.h vec3DataTransition.I vec3DataTransition.h \
    vec4DataAttribute.I vec4DataAttribute.h vec4DataTransition.I \
    vec4DataTransition.h vectorDataAttribute.I vectorDataAttribute.h \
    vectorDataTransition.I vectorDataTransition.h

//  #define PRECOMPILED_HEADER dgraph_headers.h
  
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


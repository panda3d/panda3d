#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET dgraph
  #define LOCAL_LIBS \
    sgraph graph putil sgattrib mathutil

  #define SOURCES \
    buttonEventDataAttribute.I buttonEventDataAttribute.cxx \
    buttonEventDataAttribute.h buttonEventDataTransition.I \
    buttonEventDataTransition.cxx buttonEventDataTransition.h \
    config_dgraph.cxx config_dgraph.h dataGraphTraversal.cxx \
    dataGraphTraversal.h dataNode.cxx dataNode.h dataRelation.I \
    dataRelation.N dataRelation.cxx dataRelation.h \
    describe_data_verbose.cxx describe_data_verbose.h \
    doubleDataAttribute.I doubleDataAttribute.cxx doubleDataAttribute.h \
    doubleDataTransition.I doubleDataTransition.cxx \
    doubleDataTransition.h doublePtrDataAttribute.I \
    doublePtrDataAttribute.cxx doublePtrDataAttribute.h \
    doublePtrDataTransition.I doublePtrDataTransition.cxx \
    doublePtrDataTransition.h intDataAttribute.I intDataAttribute.cxx \
    intDataAttribute.h intDataTransition.I intDataTransition.cxx \
    intDataTransition.h matrixDataAttribute.I matrixDataAttribute.cxx \
    matrixDataAttribute.h matrixDataTransition.I \
    matrixDataTransition.cxx matrixDataTransition.h \
    modifierButtonDataAttribute.I modifierButtonDataAttribute.cxx \
    modifierButtonDataAttribute.h modifierButtonDataTransition.I \
    modifierButtonDataTransition.cxx modifierButtonDataTransition.h \
    vec3DataAttribute.I vec3DataAttribute.cxx vec3DataAttribute.h \
    vec3DataTransition.I vec3DataTransition.cxx vec3DataTransition.h \
    vec4DataAttribute.I vec4DataAttribute.cxx vec4DataAttribute.h \
    vec4DataTransition.I vec4DataTransition.cxx vec4DataTransition.h

  #define INSTALL_HEADERS \
    buttonEventDataAttribute.I buttonEventDataAttribute.h \
    buttonEventDataTransition.I buttonEventDataTransition.h \
    dataGraphTraversal.h dataNode.h dataRelation.I dataRelation.h \
    describe_data_verbose.h doubleDataAttribute.I doubleDataAttribute.h \
    doubleDataTransition.I doubleDataTransition.h \
    doublePtrDataAttribute.I doublePtrDataAttribute.h \
    doublePtrDataTransition.I doublePtrDataTransition.h \
    intDataAttribute.I intDataAttribute.h intDataTransition.I \
    intDataTransition.h matrixDataAttribute.I matrixDataAttribute.h \
    matrixDataTransition.I matrixDataTransition.h \
    modifierButtonDataAttribute.I modifierButtonDataAttribute.h \
    modifierButtonDataTransition.I modifierButtonDataTransition.h \
    numericDataAttribute.I numericDataAttribute.h \
    numericDataTransition.I numericDataTransition.h \
    pointerDataAttribute.I pointerDataAttribute.h \
    pointerDataTransition.I pointerDataTransition.h vec3DataAttribute.I \
    vec3DataAttribute.h vec3DataTransition.I vec3DataTransition.h \
    vec4DataAttribute.I vec4DataAttribute.h vec4DataTransition.I \
    vec4DataTransition.h vectorDataAttribute.I vectorDataAttribute.h \
    vectorDataTransition.I vectorDataTransition.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_dgraph
  #define LOCAL_LIBS \
    dgraph

  #define SOURCES \
    test_dgraph.cxx

#end test_bin_target


#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET sgraph
  #define LOCAL_LIBS \
    gobj putil graph mathutil linmath

  #define SOURCES \
    camera.I camera.cxx camera.h config_sgraph.cxx config_sgraph.h \
    geomNode.I geomNode.cxx geomNode.h geomTransformer.I \
    geomTransformer.cxx geomTransformer.h planeNode.I planeNode.cxx \
    planeNode.h projectionNode.I projectionNode.cxx projectionNode.h \
    renderTraverser.I renderTraverser.cxx renderTraverser.h

  #define INSTALL_HEADERS \
    camera.I camera.h geomNode.I geomNode.h geomTransformer.I \
    geomTransformer.h planeNode.I planeNode.h projectionNode.I \
    projectionNode.h renderTraverser.I renderTraverser.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_sgraph
  #define LOCAL_LIBS \
    sgraph mathutil graph putil

  #define SOURCES \
    test_sgraph.cxx

#end test_bin_target


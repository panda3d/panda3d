#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET builder
  #define LOCAL_LIBS \
    linmath gobj sgraph sgattrib graph putil gsgbase mathutil pnmimage \
    pandabase

  #define SOURCES \
    builder.I builder.cxx builder.h builderAttrib.I builderAttrib.cxx \
    builderAttrib.h builderAttribTempl.I builderAttribTempl.h \
    builderBucket.I builderBucket.cxx builderBucket.h \
    builderBucketNode.I builderBucketNode.cxx builderBucketNode.h \
    builderFuncs.I builderFuncs.h builderMisc.cxx builderMisc.h \
    builderNormalVisualizer.I builderNormalVisualizer.cxx \
    builderNormalVisualizer.h builderPrim.I builderPrim.cxx builderPrim.h \
    builderPrimTempl.I builderPrimTempl.h builderProperties.cxx \
    builderProperties.h builderTypes.cxx builderTypes.h \
    builderVertex.I builderVertex.cxx builderVertex.h \
    builderVertexTempl.I builderVertexTempl.h config_builder.cxx \
    config_builder.h mesher.cxx mesher.h mesherConfig.h mesherEdge.I \
    mesherEdge.h mesherFanMaker.I mesherFanMaker.h mesherStrip.I \
    mesherStrip.h mesherTempl.I mesherTempl.h

  #define INSTALL_HEADERS \
    builder.I builder.h builderAttrib.I builderAttrib.h \
    builderAttribTempl.I builderAttribTempl.h builderBucket.I \
    builderBucket.h builderBucketNode.I builderBucketNode.h \
    builderNormalVisualizer.I builderNormalVisualizer.h \
    builderPrim.I builderPrim.h \
    builderPrimTempl.I builderPrimTempl.h builderProperties.h \
    builderTypes.h builderVertex.I builderVertex.h builderVertexTempl.I \
    builderVertexTempl.h config_builder.h

#end lib_target

#begin test_bin_target
  #define TARGET test_builder
  #define LOCAL_LIBS \
    builder

  #define SOURCES \
    test_builder.cxx test_builder_data.cxx

#end test_bin_target


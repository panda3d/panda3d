#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET builder
  #define LOCAL_LIBS \
    pgraph linmath gobj putil gsgbase mathutil pnmimage \
    pandabase

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx

  #define SOURCES \
     builder.I builder.h \
     builder_compare.I builder_compare.h \
     builderAttrib.I builderAttrib.h  \
     builderAttribTempl.I builderAttribTempl.h builderBucket.I  \
     builderBucket.h builderBucketNode.I builderBucketNode.h  \
     builderFuncs.I builderFuncs.h builderMisc.h  \
     builderNormalVisualizer.I builderNormalVisualizer.h  \
     builderPrim.I builderPrim.h builderPrimTempl.I  \
     builderPrimTempl.h builderProperties.h builderTypes.h  \
     builderVertex.I builderVertex.h builderVertexTempl.I  \
     builderVertexTempl.h config_builder.h mesher.h  \
     mesherConfig.h mesherEdge.I mesherEdge.h mesherFanMaker.I  \
     mesherFanMaker.h mesherStrip.I mesherStrip.h mesherTempl.I  \
     mesherTempl.h 
 
 #define INCLUDED_SOURCES \
     builder.cxx builderAttrib.cxx builderBucket.cxx  \
     builderBucketNode.cxx builderMisc.cxx  \
     builderNormalVisualizer.cxx builderPrim.cxx  \
     builderProperties.cxx builderTypes.cxx builderVertex.cxx  \
     config_builder.cxx mesher.cxx 

  #define INSTALL_HEADERS \
    builder.I builder.h \
    builder_compare.I builder_compare.h \
    builderAttrib.I builderAttrib.h \
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


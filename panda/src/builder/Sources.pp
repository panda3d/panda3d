#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET builder
  #define LOCAL_LIBS \
    linmath gobj sgraph sgattrib graph putil gsgbase mathutil pnmimage \
    pandabase

  #define SOURCES \
    builder.I builder.cxx builder.h builderAttrib.I builderAttrib.cxx \
    builderAttrib.h builderBucket.I builderBucket.cxx builderBucket.h \
    builderBucketNode.I builderBucketNode.cxx builderBucketNode.h \
    builderMisc.cxx builderMisc.h builderNormalVisualizer.I \
    builderNormalVisualizer.cxx builderNormalVisualizer.h \
    builderPrim.cxx builderPrim.h builderProperties.cxx \
    builderProperties.h builderTypes.cxx builderTypes.h builderVertex.I \
    builderVertex.cxx builderVertex.h config_builder.cxx \
    config_builder.h mesher.cxx mesher.h pta_BuilderC.cxx \
    pta_BuilderC.h pta_BuilderN.cxx pta_BuilderN.h pta_BuilderTC.cxx \
    pta_BuilderTC.h pta_BuilderV.cxx pta_BuilderV.h vector_BuilderC.cxx \
    vector_BuilderC.h vector_BuilderN.cxx vector_BuilderN.h \
    vector_BuilderTC.cxx vector_BuilderTC.h vector_BuilderV.cxx \
    vector_BuilderV.h

  #define INSTALL_HEADERS \
    builder.I builder.h builderAttrib.I builderAttrib.h \
    builderAttribTempl.I builderAttribTempl.h builderBucket.I \
    builderBucket.h builderBucketNode.I builderBucketNode.h \
    builderNormalVisualizer.I builderNormalVisualizer.h builderPrim.h \
    builderPrimTempl.I builderPrimTempl.h builderProperties.h \
    builderTypes.h builderVertex.I builderVertex.h builderVertexTempl.I \
    builderVertexTempl.h config_builder.h pta_BuilderC.h pta_BuilderN.h \
    pta_BuilderTC.h pta_BuilderV.h vector_BuilderC.h vector_BuilderN.h \
    vector_BuilderTC.h vector_BuilderV.h

#end lib_target

#begin test_bin_target
  #define TARGET test_builder
  #define LOCAL_LIBS \
    builder

  #define SOURCES \
    test_builder.cxx test_builder_data.cxx

#end test_bin_target


#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET sgraphutil
  #define LOCAL_LIBS \
    graph sgraph sgattrib linmath putil gobj mathutil gsgbase display \
    pnmimage

  #define SOURCES \
    appTraverser.I appTraverser.cxx appTraverser.h \
    config_sgraphutil.cxx config_sgraphutil.h directRenderTraverser.I \
    directRenderTraverser.cxx directRenderTraverser.h get_rel_pos.I \
    get_rel_pos.cxx get_rel_pos.h sceneGraphAnalyzer.cxx \
    sceneGraphAnalyzer.h sceneGraphReducer.I sceneGraphReducer.cxx \
    sceneGraphReducer.h

  #define INSTALL_HEADERS \
    appTraverser.I appTraverser.h config_sgraphutil.h \
    directRenderLevelState.h directRenderTraverser.I \
    directRenderTraverser.h frustumCullTraverser.I \
    frustumCullTraverser.h get_rel_pos.I get_rel_pos.h \
    sceneGraphAnalyzer.h sceneGraphReducer.I sceneGraphReducer.h

  #define IGATESCAN all

#end lib_target


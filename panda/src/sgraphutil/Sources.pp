#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgraphutil
  #define LOCAL_LIBS \
    pstatclient graph sgraph sgattrib linmath putil gobj mathutil \
    gsgbase display pnmimage

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
     appTraverser.I appTraverser.h config_sgraphutil.h  \
     directRenderTraverser.I directRenderTraverser.h  \
     frustumCullTraverser.I frustumCullTraverser.h get_rel_pos.I  \
     get_rel_pos.h sceneGraphAnalyzer.h sceneGraphReducer.I  \
     sceneGraphReducer.h

  #define INCLUDED_SOURCES  \
     appTraverser.cxx config_sgraphutil.cxx directRenderTraverser.cxx  \
     frustumCullTraverser.cxx get_rel_pos.cxx sceneGraphAnalyzer.cxx \
     sceneGraphReducer.cxx

  #define INSTALL_HEADERS \
    appTraverser.I appTraverser.h config_sgraphutil.h \
    directRenderLevelState.h directRenderTraverser.I \
    directRenderTraverser.h frustumCullTraverser.I \
    frustumCullTraverser.h get_rel_pos.I get_rel_pos.h \
    sceneGraphAnalyzer.h sceneGraphReducer.I sceneGraphReducer.h

  #define IGATESCAN all

#end lib_target


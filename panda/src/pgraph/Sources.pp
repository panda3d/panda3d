#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define LOCAL_LIBS gobj putil graph linmath express pandabase

#begin lib_target
  #define TARGET pgraph
  
  #define SOURCES \
    colorAttrib.h colorAttrib.I \
    config_pgraph.h \
    cullHandler.h \
    qpcullTraverser.h qpcullTraverser.I \
    cycleData.h cycleData.I \
    cycleDataReader.h cycleDataReader.I \
    cycleDataWriter.h cycleDataWriter.I \
    qpgeomNode.h qpgeomNode.I \
    pandaNode.h pandaNode.I \
    pipeline.h pipeline.I \
    pipelineCycler.h pipelineCycler.I \
    pipelineCyclerBase.h pipelineCyclerBase.I \
    renderAttrib.h renderAttrib.I \
    renderState.h renderState.I \
    textureAttrib.h textureAttrib.I \
    transformAttrib.h transformAttrib.I

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    
  #define INCLUDED_SOURCES \
    colorAttrib.cxx \
    config_pgraph.cxx \
    cullHandler.cxx \
    qpcullTraverser.cxx \
    cycleData.cxx \
    cycleDataReader.cxx \
    cycleDataWriter.cxx \
    qpgeomNode.cxx \
    pandaNode.cxx \
    pipeline.cxx \
    pipelineCycler.cxx \
    pipelineCyclerBase.cxx \
    renderAttrib.cxx \
    renderState.cxx \
    textureAttrib.cxx \
    transformAttrib.cxx

  #if $[DONT_COMBINE_PGRAPH]    
    #define SOURCES $[SOURCES] $[INCLUDED_SOURCES]
    #define INCLUDED_SOURCES
    #define COMBINED_SOURCES
  #endif

  #define INSTALL_HEADERS \
    pandaNode.h pandaNode.I

  #define IGATESCAN all

#end lib_target


#begin test_bin_target
  #define TARGET test_pgraph

  #define SOURCES \
    test_pgraph.cxx

  #define LOCAL_LIBS $[LOCAL_LIBS] pgraph
  #define OTHER_LIBS $[OTHER_LIBS] pystub

#end test_bin_target

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgraph
  #define LOCAL_LIBS \
    gobj putil graph mathutil linmath event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     camera.I camera.h config_sgraph.h geomNode.I geomNode.h  \
     geomTransformer.I geomTransformer.h modelNode.I modelNode.h  \
     modelRoot.I modelRoot.h planeNode.I planeNode.h  \
     lensNode.I lensNode.h renderTraverser.I  \
     renderTraverser.h switchNode.I switchNode.h  

  #define INCLUDED_SOURCES  \
     camera.cxx config_sgraph.cxx geomNode.cxx geomTransformer.cxx  \
     modelNode.cxx modelRoot.cxx planeNode.cxx lensNode.cxx  \
     renderTraverser.cxx switchNode.cxx 

  #define INSTALL_HEADERS \
    camera.I camera.h config_sgraph.h \
    geomNode.I geomNode.h geomTransformer.I \
    geomTransformer.h modelNode.I modelNode.h modelRoot.I \
    modelRoot.h planeNode.I planeNode.h lensNode.I \
    lensNode.h renderTraverser.I renderTraverser.h \
    switchNode.I switchNode.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_sgraph
  #define LOCAL_LIBS \
    sgraph mathutil graph putil
  #define OTHER_LIBS \
    $[OTHER_LIBS] pystub

  #define SOURCES \
    test_sgraph.cxx

#end test_bin_target


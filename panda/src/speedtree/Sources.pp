#define BUILD_DIRECTORY $[HAVE_SPEEDTREE]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES speedtree $[if $[eq $[SPEEDTREE_API],OpenGL],gl cg cggl] $[if $[eq $[SPEEDTREE_API],DirectX9],dx9 cg cgdx9]
#define BUILDING_DLL BUILDING_PANDASPEEDTREE

#begin lib_target
  #define TARGET pandaspeedtree
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_speedtree.h \
    loaderFileTypeSrt.h \
    speedtree_api.h \
    speedTreeNode.h \
    stTransform.h \
    stTree.h

  // A generated file
  #define SOURCES $[SOURCES] speedtree_parameters.h 
    
  #define INCLUDED_SOURCES \
    config_speedtree.cxx \
    loaderFileTypeSrt.cxx \
    speedtree_api.cxx \
    speedTreeNode.cxx \
    stTransform.cxx \
    stTree.cxx

  #define INSTALL_HEADERS \
    speedtree_parameters.h \
    speedtree_api.h \
    speedTreeNode.h \
    stTransform.h \
    stTree.h

  #define IGATESCAN all

#end lib_target


#include $[THISDIRPREFIX]speedtree_parameters.h.pp

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET char
  #define LOCAL_LIBS \
    chan linmath putil event mathutil gsgbase \
    pstatclient    
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    character.I character.h \
    characterJoint.h characterJointBundle.I  \
    characterJointBundle.h characterSlider.h computedVertices.I  \
    computedVertices.h computedVerticesMorph.I  \
    computedVerticesMorph.h config_char.h dynamicVertices.h \
    jointVertexTransform.I jointVertexTransform.h
    
  #define INCLUDED_SOURCES \
    character.cxx \
    characterJoint.cxx characterJointBundle.cxx  \
    characterSlider.cxx computedVertices.cxx  \
    computedVerticesMorph.cxx config_char.cxx  \
    dynamicVertices.cxx \
    jointVertexTransform.cxx

  #define INSTALL_HEADERS \
    character.I character.h \
    characterJoint.h characterJointBundle.I \
    characterJointBundle.h characterSlider.h computedVertices.I \
    computedVertices.h computedVerticesMorph.I computedVerticesMorph.h \
    config_char.h dynamicVertices.h \
    jointVertexTransform.I jointVertexTransform.h
    
  #define IGATESCAN all

#end lib_target


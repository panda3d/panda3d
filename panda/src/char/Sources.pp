#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET char
  #define LOCAL_LIBS \
    chan graph sgraph linmath putil event sgattrib mathutil gsgbase \
    pstatclient

  #define SOURCES \
    character.I character.cxx character.h characterJoint.cxx \
    characterJoint.h characterJointBundle.I characterJointBundle.cxx \
    characterJointBundle.h characterSlider.cxx characterSlider.h \
    computedVertices.I computedVertices.cxx computedVertices.h \
    computedVerticesMorph.I computedVerticesMorph.cxx \
    computedVerticesMorph.h config_char.cxx config_char.h \
    dynamicVertices.cxx dynamicVertices.h

  #define INSTALL_HEADERS \
    character.I character.h characterJoint.h characterJointBundle.I \
    characterJointBundle.h characterSlider.h computedVertices.I \
    computedVertices.h computedVerticesMorph.I computedVerticesMorph.h \
    config_char.h dynamicVertices.h

  #define IGATESCAN all

#end lib_target


#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
                   
#begin lib_target
  #define TARGET p3char
  #define LOCAL_LIBS \
    p3chan p3linmath p3putil p3event p3mathutil p3gsgbase \
    p3pstatclient    
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    character.I character.h \
    characterJoint.I characterJoint.h \
    characterJointBundle.I characterJointBundle.h \
    characterJointEffect.h characterJointEffect.I \
    characterSlider.h \
    characterVertexSlider.I characterVertexSlider.h \
    config_char.h \
    jointVertexTransform.I jointVertexTransform.h
    
  #define INCLUDED_SOURCES \
    character.cxx \
    characterJoint.cxx characterJointBundle.cxx  \
    characterJointEffect.cxx \
    characterSlider.cxx \
    characterVertexSlider.cxx \
    config_char.cxx  \
    jointVertexTransform.cxx

  #define INSTALL_HEADERS \
    character.I character.h \
    characterJoint.I characterJoint.h \
    characterJointBundle.I characterJointBundle.h \
    characterJointEffect.h characterJointEffect.I \
    characterSlider.h \
    characterVertexSlider.I characterVertexSlider.h \
    config_char.h \
    jointVertexTransform.I jointVertexTransform.h
    
  #define IGATESCAN all

#end lib_target


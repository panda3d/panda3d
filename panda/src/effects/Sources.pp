#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES cg
#begin lib_target
  #define TARGET effects
  #define LOCAL_LIBS \
    display gobj putil gsgbase linmath \
    mathutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx         

  #define SOURCES \
    config_effects.h \
    cgShader.I cgShader.h \
    cgShaderAttrib.I cgShaderAttrib.h \
    cgShaderContext.I cgShaderContext.h \
    lensFlareNode.I lensFlareNode.h
    
  #define INCLUDED_SOURCES \
    config_effects.cxx \
    cgShader.cxx \
    cgShaderAttrib.cxx \
    cgShaderContext.cxx \
    lensFlareNode.cxx 

  #define INSTALL_HEADERS \
    config_effects.h \
    cgShader.I cgShader.h \
    cgShaderAttrib.I cgShaderAttrib.h \
    cgShaderContext.I cgShaderContext.h \
    lensFlareNode.I lensFlareNode.h

  #define IGATESCAN all

#end lib_target


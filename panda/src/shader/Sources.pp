#define OTHER_LIBS dtoolconfig dtool

#define BUILDING_DLL BUILDING_SHADER

#begin lib_target
  #define TARGET shader
  #define LOCAL_LIBS \
    putil express display graph sgattrib light sgraphutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
     casterShader.I casterShader.h config_shader.h outlineShader.h  \
     planarReflector.h projtexShader.h projtexShadower.h shader.I  \
     shader.h shaderTransition.I shaderTransition.h  \
     spheretexHighlighter.h spheretexReflector.h  \
     spheretexShader.h spotlightShader.h  \

  #define INCLUDED_SOURCES  \
     casterShader.cxx config_shader.cxx outlineShader.cxx  \
     planarReflector.cxx projtexShader.cxx projtexShadower.cxx  \
     shader.cxx shaderTransition.cxx spheretexHighlighter.cxx  \
     spheretexReflector.cxx spheretexShader.cxx  \
     spotlightShader.cxx 

  #define INSTALL_HEADERS \
    casterShader.I casterShader.h outlineShader.h planarReflector.h \
    projtexShader.h projtexShadower.h shader.I shader.h \
    shaderTransition.I shaderTransition.h spheretexHighlighter.h \
    spheretexReflector.h spheretexShader.h spotlightShader.h

#end lib_target


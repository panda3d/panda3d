#define OTHER_LIBS dtool

#define BUILDING_DLL BUILDING_SHADER

#begin lib_target
  #define TARGET shader
  #define LOCAL_LIBS \
    putil express display graph sgattrib light sgraphutil

  #define SOURCES \
    casterShader.I casterShader.cxx casterShader.h config_shader.cxx \
    config_shader.h outlineShader.cxx outlineShader.h \
    planarReflector.cxx planarReflector.h projtexShader.cxx \
    projtexShader.h projtexShadower.cxx projtexShadower.h shader.I \
    shader.cxx shader.h shaderTransition.I shaderTransition.cxx \
    shaderTransition.h spheretexHighlighter.cxx spheretexHighlighter.h \
    spheretexReflector.cxx spheretexReflector.h spheretexShader.cxx \
    spheretexShader.h spotlightShader.cxx spotlightShader.h

  #define INSTALL_HEADERS \
    casterShader.I casterShader.h outlineShader.h planarReflector.h \
    projtexShader.h projtexShadower.h shader.I shader.h \
    shaderTransition.I shaderTransition.h spheretexHighlighter.h \
    spheretexReflector.h spheretexShader.h spotlightShader.h

#end lib_target


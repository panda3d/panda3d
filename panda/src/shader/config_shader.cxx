// Filename: config_shader.cxx
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_shader.h"
#include "casterShader.h"
#include "outlineShader.h"
#include "planarReflector.h"
#include "projtexShader.h"
#include "projtexShadower.h"
#include "shader.h"
#include "shaderTransition.h"
#include "spheretexHighlighter.h"
#include "spheretexReflector.h"
#include "spheretexShader.h"
#include "spotlightShader.h"

#include <dconfig.h>

Configure(config_shader);
NotifyCategoryDef(shader, "");

ConfigureFn(config_shader) {
  CasterShader::init_type();
  OutlineShader::init_type();
  PlanarReflector::init_type();
  ProjtexShader::init_type();
  ProjtexShadower::init_type();
  Shader::init_type();
  FrustumShader::init_type();
  ShaderTransition::init_type();
  SpheretexHighlighter::init_type();
  SpheretexReflector::init_type();
  SpheretexShader::init_type();
  SpotlightShader::init_type();

  //Setup shader rendering orders
  ShaderTransition::set_shader_order(ProjtexShadower::get_class_type(), 2);

  ShaderTransition::set_shader_order(OutlineShader::get_class_type(), 4);
  ShaderTransition::set_shader_order(ProjtexShader::get_class_type(), 4);
  ShaderTransition::set_shader_order(SpheretexShader::get_class_type(), 4);
  ShaderTransition::set_shader_order(SpotlightShader::get_class_type(), 4);

  ShaderTransition::set_shader_order(SpheretexHighlighter::get_class_type(), 6);

  ShaderTransition::set_shader_order(SpheretexReflector::get_class_type(), 8);

  ShaderTransition::set_shader_order(PlanarReflector::get_class_type(), 10);

  //Tell shaderTransition the shaders that require blending
  ShaderTransition::set_shader_always_blend(SpotlightShader::get_class_type());
  ShaderTransition::set_shader_always_blend(SpheretexHighlighter::get_class_type());
}

// Filename: spotlightShader.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "spotlightShader.h"
#include "config_shader.h"

#include <spotlight.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle SpotlightShader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SpotlightShader::constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
SpotlightShader::SpotlightShader(int size, float radius) : ProjtexShader(NULL, ColorBlendProperty::M_multiply_add)
{
  Texture* texture = new Texture;
  texture->set_minfilter(Texture::FT_linear);
  texture->set_magfilter(Texture::FT_linear);
  texture->set_wrapu(Texture::WM_clamp);
  texture->set_wrapv(Texture::WM_clamp);
  texture->_pbuffer = new PixelBuffer(PixelBuffer::rgb_buffer(size, size));
  set_texture(texture);
  _radius = radius;
}

////////////////////////////////////////////////////////////////////
//     Function: SpotlightShader::config 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void SpotlightShader::config(void)
{
  Configurable::config();

  if (get_num_frusta() == 0) {
    shader_cat.error()
      << "SpotlightShader::config() - no lights in frusta list" << endl;
    return;
  } else if (get_num_frusta() > 1) {
    shader_cat.warning()
      << "SpotlightShader::config() - frusta list has more than one "
      << "frustum - ignore all but the first one for now..." << endl;
  }
  if (_frusta[0]->get_type() != Spotlight::get_class_type()) {
    shader_cat.error()
      << "SpotlightShader::config() - frusta is not a spotlight" << endl;
    return;
  }
  Spotlight* spotlight = (Spotlight *)_frusta[0];
  
  spotlight->make_image(_texture, _radius);
}






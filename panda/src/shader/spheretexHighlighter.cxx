// Filename: spheretexHighlighter.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////
#include "spheretexHighlighter.h"
#include "config_shader.h"

#include "light.h"
#include "spotlight.h"
#include "renderBuffer.h"
#include "get_rel_pos.h"
#include "dftraverser.h"
#include "displayRegion.h"
#include "graphicsWindow.h"
#include "materialTransition.h"
#include "materialAttribute.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle SpheretexHighlighter::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
SpheretexHighlighter::SpheretexHighlighter(int size) : FrustumShader()
{
  set_size(size);

  Texture* texture = new Texture;
  texture->set_minfilter(Texture::FT_linear);
  texture->set_magfilter(Texture::FT_linear);
  texture->set_wrapu(Texture::WM_clamp);
  texture->set_wrapv(Texture::WM_clamp);
  texture->_pbuffer = new PixelBuffer(PixelBuffer::rgb_buffer(_size, _size));
  _spheretex_shader.set_texture(texture);
  _spheretex_shader.set_blend_mode(ColorBlendProperty::M_add);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::set_multipass
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexHighlighter::
set_multipass(bool)
{
  //A highlight shader always needs to blend
  _multipass_on = true;
 _spheretex_shader.set_multipass(true);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::set_priority
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexHighlighter::
set_priority(int priority)
{
  Shader::set_priority(priority);
  _spheretex_shader.set_priority(priority);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::pre_apply
//       Access:
//  Description:
///////////////////////////////////////////////////////////////////
void SpheretexHighlighter::
pre_apply(Node *node, const AllAttributesWrapper &init_state,
          const AllTransitionsWrapper &, GraphicsStateGuardian *gsg)
{
  if (get_num_frusta() == 0) {
    shader_cat.error()
      << "SpheretexHighlighter::config() - no lights in frusta list"
      << endl;
    return;
  } else if (get_num_frusta() > 1) {
    shader_cat.warning()
      << "SpheretexHighlighter::config() - frusta list has more than one "
      << "frustum - ignore all but the first one for now..." << endl;
  }
  if (_frusta[0]->get_type() != Spotlight::get_class_type()) {
    shader_cat.error()
      << "SpheretexHighlighter::config() - only works for Spotlights"
      << " so far - we'll add point lights later" << endl;
    return;
  }
  // MPG - we could make this work for point lights as well
  Spotlight* light = (Spotlight *)_frusta[0];

  // Figure out how shiny the highlighted object is
  float shininess = 0.0;

  const NodeAttribute *mat_attrib =
    init_state.get_attribute(MaterialTransition::get_class_type());
  if (mat_attrib != (NodeAttribute *)NULL) {
    const Material *material =
      DCAST(MaterialAttribute, mat_attrib)->get_material();
    shininess = material->get_shininess();
  }

  if (shininess == 0.0) {
    shininess = 100.0;
  }

  // Figure out where the highlight is relative to the object's center and
  // the light
  const LensNode* projnode = gsg->get_current_camera();
  LVector3f model_pos = get_rel_pos(node, projnode);
  LVector3f norm_model_pos = normalize(model_pos);
  LPoint3f light_pos = get_rel_pos(light, projnode);
  LVector3f light_vec = light_pos - model_pos;
  light_vec = normalize(light_vec);

  // Compute highlight for all pixels on the image that are on the sphere
  LVector3f view_vec(0, -1, 0); // Looking down +y axis
  LVector3f norm, S;
  float ZZ, XX_ZZ, intensity, grazing;
  uchar color[3];
  float scale = 1.0 / ((float)(_size - 1));

  Texture* texture = _spheretex_shader.get_texture();

  for (int y = 0; y < _size; ++y) {
    norm[2] = 2. * ((float)y * scale) - 1.; // 0.._size-1 -> -1..1
    ZZ = norm[2] * norm[2]; // z^2

    for (int x = 0; x < _size; ++x) {
      norm[0] = 2. * ((float)x * scale) -1.; // 0.._size-1 -> -1..1
      XX_ZZ = (norm[0] * norm[0]) + ZZ; // x^2 + z^2

      color[0] = color[1] = color[2] = 0;

      if (XX_ZZ <= 1.) { // Are we on the unit sphere?
        norm[1] = -(float)sqrt(1. - XX_ZZ); // This yields a unit vector
        grazing = dot(norm, light_vec);
        if (grazing > 0.) { // Does any light actually reflect off this point?
          S = norm;
          S *= 2. * grazing;
          S = S - light_vec; // the reflection vector
          intensity = -dot(S, norm_model_pos); // approximate R . V
          // If, relative to the viewer, the normal doesn't point towards the
          // light (even slightly), no reflection will be seen
          if (intensity > 0.) {
            intensity = pow(intensity, shininess);
            color[0] = color[1] = color[2] = (uchar)(intensity * 255.);
          }
        }
      }
      texture->_pbuffer->set_uchar_rgb_texel(color, x, y, _size);
      texture->unprepare();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexHighlighter::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg) {

  Shader::apply(node, init_state, net_trans, gsg);

  _spheretex_shader.apply(node, init_state, net_trans, gsg);
}

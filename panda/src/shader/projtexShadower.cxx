// Filename: projtexShadower.cxx
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
#include "projtexShadower.h"
#include "config_shader.h"

#include "light.h"
#include "spotlight.h"
#include "renderBuffer.h"
#include "transformTransition.h"
#include "lightTransition.h"
#include "textureTransition.h"
#include "colorBlendTransition.h"
#include "colorTransition.h"
#include "depthTestTransition.h"
#include "depthWriteTransition.h"
#include "polygonOffsetTransition.h"
#include "get_rel_pos.h"
#include "dftraverser.h"
#include "displayRegion.h"
#include "graphicsWindow.h"
#include "directRenderTraverser.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ProjtexShadower::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShadower::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
ProjtexShadower::ProjtexShadower(int size) : CasterShader()
{
  set_size(size);

  Texture* texture = new Texture;
  texture->set_minfilter(Texture::FT_linear);
  texture->set_magfilter(Texture::FT_linear);
  texture->set_wrapu(Texture::WM_clamp);
  texture->set_wrapv(Texture::WM_clamp);
  texture->_pbuffer->set_xsize(_size);
  texture->_pbuffer->set_ysize(_size);

  _projtex_shader = new ProjtexShader(texture, ColorBlendProperty::M_multiply);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShadower::set_priority
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ProjtexShadower::
set_priority(int priority)
{
  Shader::set_priority(priority);
  _projtex_shader->set_priority(priority);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShadower::set_multipass
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ProjtexShadower::
set_multipass(bool on)
{
  Shader::set_multipass(on);
  _projtex_shader->set_multipass(on);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShadower::pre_apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ProjtexShadower::
pre_apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg)
{
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());

  int num_lights = 1;

  if (get_num_casters() == 0) {
    shader_cat.error()
      << "ProjtexShadower::config() - no casters in caster list" << endl;
    return;
  }
  if (get_num_frusta() == 0) {
    shader_cat.error()
      << "ProjtexShadower::config() - no lights in frusta list" << endl;
    return;
  } else if (get_num_frusta() > 1) {
    shader_cat.warning()
      << "ProjtexShadower::config() - frusta list has more than one "
      << "frustum - ignore all but the first one for now..." << endl;
    num_lights = 1;
  }
  if (_frusta[0]->get_type() != Spotlight::get_class_type()) {
    shader_cat.error()
      << "ProjtexShadower::config() - only works for Spotlights" << endl;
    return;
  }
  Spotlight* light = (Spotlight *)_frusta[0];

  // Save the current display region from the gsg
  Colorf clear_color = gsg->get_color_clear_value();

  // Make sure the projtex shader has the same frustum
  _projtex_shader->remove_frustum(light);
  _projtex_shader->add_frustum(light);

  // Compute an approximation for the shadow color
  Colorf diffuse = light->get_color();
  Colorf shadow_color;
  for (int i = 0; i < 4; i++) {
    shadow_color[i] = 1.0 / ((diffuse[i] + 1) * num_lights);
  }

  nassertv(node != (Node *)NULL && gsg != (GraphicsStateGuardian *)NULL);

  // First, draw the receiving node quickly (with no rendering attributes)
  // from the point of view of the light

  // Make a display region of the proper size and clear it to prepare for
  // rendering the shadow map
  PT(DisplayRegion) disp_region =
        gsg->get_window()->make_scratch_display_region(_size, _size);
  DisplayRegionStack old_dr = gsg->push_display_region(disp_region);

  // Save scratch region before clearing it to be non-destructive
  FrameBufferStack old_fb = gsg->push_frame_buffer
    (gsg->get_render_buffer(RenderBuffer::T_back | RenderBuffer::T_depth),
     disp_region);

  gsg->set_color_clear_value(Colorf(1., 1., 1., 1.));
  gsg->clear(gsg->get_render_buffer(RenderBuffer::T_back |
                                      RenderBuffer::T_depth), disp_region);

  // Copy the transition wrapper so we can modify it freely.
  AllTransitionsWrapper trans(net_trans);

  // Disable lighting, texturing, blending, dithering
  trans.set_transition(new LightTransition(LightTransition::all_off()));
  //The 100 is to set the priority of this transition high enough
  //(hopefully) to turn off any existing texture in the geometry
  TextureTransition *t = new TextureTransition(TextureTransition::off());
  t->set_priority(100);
  trans.set_transition(t);
  trans.set_transition(new ColorBlendTransition(ColorBlendProperty::M_none));


  //We need a state that we can modify to remove the camera transform
  //that is passed to us
  AllAttributesWrapper state(init_state);
  state.clear_attribute(TransformTransition::get_class_type());

  // Render the node with the new transitions from the viewpoint of the light

  gsg->render_subgraph(&drt, node, light, state, trans);

  // Now draw each of the casting objects in the shadow color using the
  // depth buffer from the receiving object (this will project the shadows
  // onto the surface properly), and use this to generate the shadow texture
  // map

  DepthTestTransition *dta =
    new DepthTestTransition(DepthTestProperty::M_less);
  trans.set_transition(dta);

  DepthWriteTransition *dwa = new DepthWriteTransition;
  trans.set_transition(dwa);

  ColorTransition *ca = new ColorTransition(shadow_color);
  trans.set_transition(ca);

  // Create a temporary node that is the parent of all of the caster
  // objects, so we can render them from the point of view of the
  // light.

  PT_Node caster_group = new Node;
  NamedNodeVector::iterator ci;
  for (ci = _casters.begin(); ci != _casters.end(); ++ci) {
    Node *caster = (*ci);

    LMatrix4f mat;
    get_rel_mat(caster, node, mat);

    RenderRelation *arc = new RenderRelation(caster_group, caster);
    arc->set_transition(new TransformTransition(mat));
  }

  // Parent this caster group to the node itself, since it's already
  // converted to the coordinate space of the node
  PT(RenderRelation) caster_arc = new RenderRelation(node, caster_group);

  // Now render the group.
  gsg->render_subgraph(&drt, caster_group, light, state, trans);

  // Remove the caster group from the scene graph.  This will also
  // delete the temporary group node (since the reference count is
  // zero), but not the caster objects themselves.
  remove_arc(caster_arc);

  // Copy the results of the render from the framebuffer into the projtex
  // shader's texture
  // _projtex_shader->get_texture()->prepare(gsg);
  _projtex_shader->get_texture()->copy(gsg, disp_region, gsg->get_render_buffer(RenderBuffer::T_back));

  // Restore the original display region and clear value

  gsg->set_color_clear_value(clear_color);
  gsg->pop_frame_buffer(old_fb);
  gsg->pop_display_region(old_dr);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjtexShadower::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void ProjtexShadower::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg) {

  Shader::apply(node, init_state, net_trans, gsg);

  // Now apply the projected shadow map
  _projtex_shader->apply(node, init_state, net_trans, gsg);
}














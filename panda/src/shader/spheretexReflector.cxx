// Filename: spheretexReflector.cxx
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
#include "spheretexReflector.h"
#include "config_shader.h"

#include "pt_Node.h"
#include "frustum.h"
#include "get_rel_pos.h"
#include "dftraverser.h"
#include "displayRegion.h"
#include "graphicsWindow.h"
#include "renderBuffer.h"
#include "perspectiveLens.h"
#include "look_at.h"
#include "cullFaceTransition.h"
#include "colorBlendTransition.h"
#include "transformTransition.h"
#include "directRenderTraverser.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle SpheretexReflector::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SpheretexReflector::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
SpheretexReflector::SpheretexReflector(int size) : CasterShader()
{
  set_size(size);
  _is_reflector = true;

  _fnear = 0.1f;
  _ffar = 1000.0f;

  Texture* texture = new Texture;
  texture->set_minfilter(Texture::FT_linear);
  texture->set_magfilter(Texture::FT_linear);
  texture->set_wrapu(Texture::WM_clamp);
  texture->set_wrapv(Texture::WM_clamp);
  texture->_pbuffer->set_xsize(_size);
  texture->_pbuffer->set_ysize(_size);

  _spheretex_shader.set_texture(texture);
  _spheretex_shader.set_blend_mode(ColorBlendProperty::M_add);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexReflector::destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
SpheretexReflector::~SpheretexReflector(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexReflector::set_priority
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexReflector::
set_priority(int priority)
{
  Shader::set_priority(priority);
  _spheretex_shader.set_priority(priority);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexReflector::set_multipass
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexReflector::
set_multipass(bool on)
{
  Shader::set_multipass(on);
  _spheretex_shader.set_multipass(on);
}

////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::pre_apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexReflector::
pre_apply(Node *node, const AllAttributesWrapper &init_state,
          const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg)
{
  if (get_num_casters() == 0) {
    shader_cat.error()
      << "SpheretexReflector::config() - no casters in caster list" << endl;
    return;
  }

  //We need a state that we can modify to remove the camera transform
  //that is passed to us
  AllAttributesWrapper state(init_state);
  state.clear_attribute(TransformTransition::get_class_type());

  // Get the current camera from the gsg
  const LensNode* camera = gsg->get_current_camera();

  // Save the clear color.
  Colorf clear_color = gsg->get_color_clear_value();

  // Make a display region of the proper size and clear it to prepare for
  // rendering the shadow map
  PT(DisplayRegion) scratch_region =
    gsg->get_window()->make_scratch_display_region(_size, _size);

  DisplayRegionStack old_dr = gsg->push_display_region(scratch_region);

  // Save scratch region before clearing it to be non-destructive
  FrameBufferStack old_fb = gsg->push_frame_buffer
    (gsg->get_render_buffer(RenderBuffer::T_back | RenderBuffer::T_depth),
     scratch_region);

  gsg->set_color_clear_value(Colorf(0., 0., 0., 1.));
  gsg->clear(gsg->get_render_buffer(RenderBuffer::T_back |
                                    RenderBuffer::T_depth), scratch_region);

  // Copy the transition wrapper so we can modify it freely.
  AllTransitionsWrapper trans(net_trans);

  // Create a temporary node that is the parent of all of the caster
  // objects, so we can render them from the point of view of the
  // reflector.

  PT_Node caster_group = new Node;
  NamedNodeVector::iterator ci;
  for (ci = _casters.begin(); ci != _casters.end(); ++ci) {
    Node *caster = (*ci);

    // Get the transform of the caster from the point of view of the
    // reflector.
    LMatrix4f mat;
    get_rel_mat(caster, node, mat);

    RenderRelation *arc = new RenderRelation(caster_group, caster);
    arc->set_transition(new TransformTransition(mat));
  }

  // Build a LensNode that represents a wide-angle view from the
  // reflecting object towards the camera.
  PT(Lens) *lens = new PerspectiveLens;
  lens->set_fov(150.0f);
  lens->set_near(_fnear);
  lens->set_far(_ffar);
  LensNode *projnode = new LensNode;
  projnode->set_lens(lens);

  // How shall we rotate the LensNode?
  LMatrix4f r_mat;

  // Get camera pos in the coordinate space of the reflecting object
  LVector3f camera_pos = get_rel_pos(camera, node);
  LVector3f camera_up = get_rel_up(camera, node);
  if (_is_reflector == true)
  {
    // Take the picture looking toward the camera
    look_at(r_mat, camera_pos, camera_up, gsg->get_coordinate_system());

    // Flip along the X axis to make it behave like a mirror
    r_mat = LMatrix4f::scale_mat(LVector3f(-1.0f, 1.0f, 1.0f)) * r_mat;

    // And the vertex order for front facing polygons to compensate
    // for the above flip.
    CullFaceTransition *cf =
      new CullFaceTransition(CullFaceProperty::M_cull_counter_clockwise);
    trans.set_transition(cf);
  }
  else
  {
    // For a refractor, take the picture looking away from the camera.
    look_at(r_mat, -camera_pos, camera_up, gsg->get_coordinate_system());
  }

  RenderRelation *projarc = new RenderRelation(caster_group, projnode);
  projarc->set_transition(new TransformTransition(r_mat));

  // Now render the casters.
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());
  gsg->render_subgraph(&drt, caster_group, projnode, state, trans);

  // The caster group, and all of the temporary group nodes below,
  // will automatically be destructed when the caster_group PointerTo
  // object goes out of scope.  We don't have to remove it from the
  // scene graph because we never actually inserted it.

  // Copy the results of the render from the framebuffer into the spheretex
  // shader's texture
  _spheretex_shader.get_texture()->copy(gsg, scratch_region, gsg->get_render_buffer(RenderBuffer::T_back));

  // Restore the original display region and clear value

  //Make sure the restore the depth buffer.  There's only one you know.

  gsg->set_color_clear_value(clear_color);
  gsg->pop_frame_buffer(old_fb);
  gsg->pop_display_region(old_dr);
}
////////////////////////////////////////////////////////////////////
//     Function: SpheretexHighlighter::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void SpheretexReflector::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg)
 {
  Shader::apply(node, init_state, net_trans, gsg);

  // Apply the spheremap reflection
  //gsg->prepare_display_region();
  _spheretex_shader.apply(node, init_state, net_trans, gsg);
}


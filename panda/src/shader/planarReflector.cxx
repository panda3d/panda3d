// Filename: planarReflector.cxx
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
#include "planarReflector.h"
#include "config_shader.h"

#include "pt_Node.h"
#include "dftraverser.h"
#include "attribTraverser.h"
#include "displayRegion.h"
#include "graphicsWindow.h"
#include "renderBuffer.h"
#include "perspectiveProjection.h"
#include "look_at.h"
#include "get_rel_pos.h"
#include "lightTransition.h"
#include "depthTestTransition.h"
#include "depthWriteTransition.h"
#include "depthWriteTransition.h"
#include "textureTransition.h"
#include "stencilTransition.h"
#include "colorBlendTransition.h"
#include "colorBlendAttribute.h"
#include "cullFaceTransition.h"
#include "transformTransition.h"
#include "colorMaskTransition.h"
#include "colorTransition.h"
#include "clipPlaneTransition.h"
#include "directRenderTraverser.h"
#include "nodeAttributes.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle PlanarReflector::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
PlanarReflector::PlanarReflector(void) : CasterShader()
{
  Colorf c(0.8f, 0.8f, 0.8f, 1.0f);
  init(NULL, c);
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
PlanarReflector::
PlanarReflector(PlaneNode* plane_node) : CasterShader()
{
  Colorf c(0.8f, 0.8f, 0.8f, 1.0f);
  init(plane_node, c);
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
PlanarReflector::
PlanarReflector(const Colorf& c) : CasterShader()
{
  init(NULL, c);
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
PlanarReflector::
PlanarReflector(PlaneNode* plane_node, const Colorf& c) : CasterShader()
{
  init(plane_node, c);
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::init
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void PlanarReflector::init(PlaneNode *plane_node, const Colorf& c)
{
  if (plane_node == NULL)
    plane_node = new PlaneNode;

  _save_color_buffer = true;
  _save_depth_buffer = true;
  _clip_to_plane = true;

  _color_buffer = NULL;
  _depth_buffer = NULL;

  _plane_node = plane_node;
  _color = c;
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::pre_apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void PlanarReflector::
pre_apply(Node *, const AllAttributesWrapper &,
          const AllTransitionsWrapper &, GraphicsStateGuardian *gsg)
{
  int xo, yo, w, h;
  gsg->get_current_display_region()->get_region_pixels(xo, yo, w, h);
  _color_buffer = new PixelBuffer(PixelBuffer::rgb_buffer(w, h));
  _depth_buffer = new PixelBuffer(PixelBuffer::depth_buffer(w, h));
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarReflector::apply
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void PlanarReflector::
apply(Node *node, const AllAttributesWrapper &init_state,
      const AllTransitionsWrapper &net_trans, GraphicsStateGuardian *gsg) {
  DirectRenderTraverser drt(gsg, RenderRelation::get_class_type());

  Shader::apply(node, init_state, net_trans, gsg);

  init_state.write(shader_cat->debug(false), 4);

  if (get_num_casters() == 0) {
    shader_cat.error()
      << "PlanarReflector::apply() - no casters in caster list" << endl;
    return;
  }

  //Due to possible priority problems, make an Off TextureTransition
  //with a high priority to ensure that textures are indeed turned of
  PT(TextureTransition) tex_off = new TextureTransition(TextureTransition::off());
  tex_off->set_priority(100);

  // If the node is un-textured, we need to render it once first (since
  // the shader transition won't have drawn it unless it is textured)

  if (!_multipass_on) {
    gsg->render_subgraph(&drt, node, init_state, net_trans);
  }

  // Save the stencil buffer clear value for below
  bool clear_stencil = gsg->get_stencil_clear_value();
  gsg->set_stencil_clear_value(false);

  {
    // Copy the transition wrapper so we can modify it freely.
    AllTransitionsWrapper trans(net_trans);

    // Save the current fully-rendered scene that is in the back buffer
    int buffer_mask = RenderBuffer::T_stencil;
    if (_save_color_buffer) {
      gsg->copy_pixel_buffer(_color_buffer,
                             gsg->get_current_display_region(),
                             gsg->get_render_buffer(RenderBuffer::T_back));
      buffer_mask |= RenderBuffer::T_back;

      ColorMaskTransition *cm = new ColorMaskTransition(0);
      trans.set_transition(cm);
    } else {
      ColorMaskTransition *cm =
        new ColorMaskTransition(ColorMaskProperty::M_a);
      trans.set_transition(cm);
    }

    if (_save_depth_buffer)
    {
      gsg->copy_pixel_buffer(_depth_buffer,
                             gsg->get_current_display_region(),
                             gsg->get_render_buffer(RenderBuffer::T_depth));
      //                     gsg->get_render_buffer(RenderBuffer::T_back));
    }

    // The scene has already been rendered so we need to stencil in an area
    // on the reflecting plane that is covered by the reflected objects.

    // Turn lighting off
    trans.set_transition(new LightTransition(LightTransition::all_off()));

    // Set the depth test to M_equal (? Or should this be M_none?)
    DepthTestTransition *dta =
      new DepthTestTransition(DepthTestProperty::M_equal);
    trans.set_transition(dta);

    // Turn off writes to the depth buffer
    DepthWriteTransition *dwa = new DepthWriteTransition;
    dwa->set_off();
    trans.set_transition(dwa);

    // Enable the stencil buffer
    StencilTransition *sa =
      new StencilTransition(StencilProperty::M_not_equal,
                            StencilProperty::A_replace);
    trans.set_transition(sa);

    // Disable texturing
    trans.set_transition(tex_off);

    // Disable blending
    trans.set_transition(new ColorBlendTransition(ColorBlendProperty::M_none));

    // Clear the stencil buffer (and color buffer if we're saving it)
    gsg->clear(gsg->get_render_buffer(buffer_mask));

    // Draw the reflecting object
    gsg->render_subgraph(&drt, node, init_state, trans);
  }

  {
    // Reflecting area on the plane has a stencil value of 1.  We can now
    // draw the reflected objects into this area.
    gsg->clear(gsg->get_render_buffer(RenderBuffer::T_depth));

    // Copy the transition wrapper so we can modify it freely.
    AllTransitionsWrapper trans(net_trans);

    // Adjust the stencil buffer properties
    StencilTransition *sa =
      new StencilTransition(StencilProperty::M_equal,
                            StencilProperty::A_keep);
    trans.set_transition(sa);


    // Draw back facing polys only
    CullFaceTransition *cf =
      new CullFaceTransition(CullFaceProperty::M_cull_counter_clockwise);
    trans.set_transition(cf);

    if (_clip_to_plane) {
      // Clip to the indicated plane.
      ClipPlaneTransition *cp = new ClipPlaneTransition;
      cp->set_on(_plane_node);
      trans.set_transition(cp);
    }

    // Reflect objects about the given plane
    PT_Node caster_group = new Node;
    NamedNodeVector::iterator c;
    for (c = _casters.begin(); c != _casters.end(); ++c) {
      Node *caster = (*c);

      LMatrix4f mat;
      get_rel_mat(caster, _plane_node, mat);

      RenderRelation *arc = new RenderRelation(caster_group, caster);
      arc->set_transition(new TransformTransition(mat));
    }

    // Parent this caster group to the node itself, since it's already
    // converted to the coordinate space of the node.
    PT(RenderRelation) caster_arc =
      new RenderRelation(_plane_node, caster_group);

    LMatrix4f plane_mat = _plane_node->get_plane().get_reflection_mat();
    caster_arc->set_transition(new TransformTransition(plane_mat));

    // Draw the reflected objects
    gsg->render_subgraph(&drt, _plane_node, init_state, trans);

    // Remove the caster group from the scene graph.  This will also
    // delete the temporary group node (since the reference count is
    // zero), but not the caster objects themselves.
    remove_arc(caster_arc);
  }

  {
    // Draw the reflecting object once more to modulate the reflected objects
    // by the reflectivity color
    AllTransitionsWrapper trans(net_trans);

    trans.set_transition(new StencilTransition(StencilProperty::M_none));

    ColorBlendTransition *cb =
      new ColorBlendTransition(ColorBlendProperty::M_multiply);
    trans.set_transition(cb);

    ColorTransition *c = new ColorTransition;
    c->set_on(_color);
    trans.set_transition(c);

    trans.set_transition(new LightTransition(LightTransition::all_off()));
    trans.set_transition(tex_off);

    gsg->render_subgraph(&drt, node, init_state, trans);
  }

  // Blend the previously rendered image with the reflected image

  if (_save_color_buffer) {
    NodeAttributes na;

    ColorBlendAttribute *cb =
      new ColorBlendAttribute;
    cb->set_mode(ColorBlendProperty::M_add);
    na.set_attribute(ColorBlendTransition::get_class_type(), cb);

    gsg->draw_pixel_buffer(_color_buffer, gsg->get_current_display_region(),
                           gsg->get_render_buffer(RenderBuffer::T_back), na);
  } else {
    // One more pass to redraw the reflecting object (additive blending)
    // Final color is reflecting obj color + (ref. obj. color * reflectivity)
    AllTransitionsWrapper trans(net_trans);

    StencilTransition *sa =
      new StencilTransition(StencilProperty::M_equal,
                            StencilProperty::A_keep);
    trans.set_transition(sa);

    DepthTestTransition *dta =
      new DepthTestTransition(DepthTestProperty::M_equal);
    trans.set_transition(dta);

    DepthWriteTransition *dwa = new DepthWriteTransition;
    dwa->set_off();
    trans.set_transition(dwa);

    ColorBlendTransition *cb =
      new ColorBlendTransition(ColorBlendProperty::M_add);
    trans.set_transition(cb);

    gsg->render_subgraph(&drt, node, init_state, trans);
  }

  if (_save_depth_buffer)
  {
    gsg->draw_pixel_buffer(_depth_buffer, gsg->get_current_display_region(),
                           gsg->get_render_buffer(RenderBuffer::T_depth));
              //           gsg->get_render_buffer(RenderBuffer::T_back));
  }


  // Restore the stencil buffer clear value
  gsg->set_stencil_clear_value(clear_stencil);
}





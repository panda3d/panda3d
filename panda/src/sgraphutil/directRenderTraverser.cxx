// Filename: directRenderTraverser.cxx
// Created by:  drose (18Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "directRenderTraverser.h"
#include "config_sgraphutil.h"
#include "frustumCullTraverser.h"

#include <wrt.h>
#include <geomNode.h>
#include <graphicsStateGuardian.h>
#include <displayRegion.h>
#include <geometricBoundingVolume.h>
#include <projectionNode.h>
#include <projection.h>
#include <colorTransition.h>
#include <renderModeTransition.h>
#include <textureTransition.h>
#include <transformTransition.h>
#include <allAttributesWrapper.h>
#include <allTransitionsWrapper.h>
#include <transformTransition.h>
#include <nodeTransitionWrapper.h>
#include <switchNode.h>
#include <decalTransition.h>
#include <decalAttribute.h>
#include <config_sgattrib.h>   // for support_decals
#include <pStatTimer.h>

TypeHandle DirectRenderTraverser::_type_handle;

#ifndef CPPPARSER
PStatCollector DirectRenderTraverser::_draw_pcollector("Draw");
#endif

////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTraverser::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DirectRenderTraverser::
DirectRenderTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
                      const ArcChain &arc_chain) :
  RenderTraverser(gsg, graph_type, arc_chain)
{
}


////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTraverser::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
DirectRenderTraverser::
~DirectRenderTraverser() {
}


////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTraverser::traverse
//       Access: Public, Virtual
//  Description: This performs a normal, complete render traversal
//               using this DirectRenderTraverser object.
////////////////////////////////////////////////////////////////////
void DirectRenderTraverser::
traverse(Node *root,
         const AllAttributesWrapper &initial_state,
         const AllTransitionsWrapper &net_trans) {
  // Statistics
  PStatTimer timer(_draw_pcollector);

  AllAttributesWrapper render_state;
  render_state.apply_from(initial_state, net_trans);

  DirectRenderLevelState level_state;

#ifndef NDEBUG
  if (support_decals != SD_off)
#endif
    {
      DecalAttribute *decal_attrib;
      if (get_attribute_into(decal_attrib, render_state,
                             DecalTransition::get_class_type())) {
        level_state._decal_mode = decal_attrib->is_on();
      }
    }

  // Determine the relative transform matrix from the camera to our
  // starting node.  This is important for proper view-frustum
  // culling.
  LMatrix4f rel_from_camera;
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  const DisplayRegion *dr = _gsg->get_current_display_region();
  ProjectionNode *camera = dr->get_cull_frustum();
  wrt(camera, root, begin(), end(), ntw, get_graph_type());
  const TransformTransition *tt;
  if (get_transition_into(tt, ntw)) {
    rel_from_camera = tt->get_matrix();
  } else {
    // No relative transform.
    rel_from_camera = LMatrix4f::ident_mat();
  }

  fc_traverse(_arc_chain, root, rel_from_camera, *this,
              render_state, level_state, _gsg, _graph_type);

  if (level_state._decal_mode &&
      root->is_of_type(GeomNode::get_class_type())) {
#ifndef NDEBUG
    if (support_decals == SD_hide) {
      return;
    }
#endif
    // Close the decal.
    _gsg->end_decal(DCAST(GeomNode, root));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTraverser::reached_node
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool DirectRenderTraverser::
reached_node(Node *node, AllAttributesWrapper &render_state,
             DirectRenderLevelState &level_state) {
  if (implicit_app_traversal) {
    node->app_traverse();
  }
  node->draw_traverse();

  level_state._decal_mode = false;

  AllTransitionsWrapper new_trans;

  _gsg->_nodes_pcollector.add_level(1);

#ifndef NDEBUG
  if (support_subrender == SD_on)
#endif
    {
      if (!node->sub_render(render_state, new_trans, this)) {
        return false;
      }
      render_state.apply_in_place(new_trans);
    }

  if (node->is_of_type(GeomNode::get_class_type())) {
    _gsg->_geom_nodes_pcollector.add_level(1);
    _gsg->set_state(render_state.get_attributes(), true);
    // Make sure the current display region is still in effect.
    _gsg->prepare_display_region();

    GeomNode *geom = DCAST(GeomNode, node);

    // We must make decals a special case, because they're so strange.
#ifndef NDEBUG
    if (support_decals != SD_off)
#endif
      {
        DecalAttribute *decal_attrib;
        if (get_attribute_into(decal_attrib, render_state,
                               DecalTransition::get_class_type())) {
          level_state._decal_mode = decal_attrib->is_on();
        }
      }

    if (level_state._decal_mode) {
#ifndef NDEBUG
      if (support_decals == SD_hide) {
        level_state._decal_mode = false;
        geom->draw(_gsg);
        return false;
      }
#endif
      _gsg->begin_decal(geom);

    } else {
      geom->draw(_gsg);
    }

  } else if (node->is_of_type(SwitchNode::get_class_type())) {
    SwitchNode *swnode = DCAST(SwitchNode, node);
    swnode->compute_switch(this);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTraverser::forward_arc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool DirectRenderTraverser::
forward_arc(NodeRelation *arc, AllTransitionsWrapper &trans,
            AllAttributesWrapper &, AllAttributesWrapper &post,
            DirectRenderLevelState &) {
  bool carry_on = true;

  // Go through all the transitions on the arc and sub_render() on
  // each one.  For most transition types, this will be a no-op and
  // return true.  For Shader types, this will fire off another render
  // and return false.

  mark_forward_arc(arc);

#ifndef NDEBUG
  if (support_subrender == SD_on)
#endif
    {
      AllTransitionsWrapper::const_iterator nti;
      for (nti = trans.begin(); nti != trans.end(); ++nti) {
        NodeTransition *t = (*nti).second.get_trans();
        AllTransitionsWrapper new_trans;
        if (!t->sub_render(arc, post, new_trans, this)) {
          carry_on = false;
        }
        post.apply_in_place(new_trans);
      }
    }

  if (!carry_on) {
    mark_backward_arc(arc);
  }

  return carry_on;
}


////////////////////////////////////////////////////////////////////
//     Function: DirectRenderTraverser::backward_arc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void DirectRenderTraverser::
backward_arc(NodeRelation *arc, AllTransitionsWrapper &,
             AllAttributesWrapper &, AllAttributesWrapper &post,
             const DirectRenderLevelState &level_state) {
  mark_backward_arc(arc);
  if (level_state._decal_mode) {
    // Reset the state to redraw the base geometry.
    _gsg->set_state(post.get_attributes(), true);
    _gsg->prepare_display_region();
    _gsg->end_decal(DCAST(GeomNode, arc->get_child()));
  }
}


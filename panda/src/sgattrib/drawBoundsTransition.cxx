// Filename: drawBoundsTransition.cxx
// Created by:  drose (26Jun00)
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

#include "config_sgattrib.h"
#include "drawBoundsTransition.h"
#include "renderModeAttribute.h"
#include "renderModeTransition.h"
#include "cullFaceAttribute.h"
#include "cullFaceTransition.h"
#include "colorAttribute.h"
#include "colorTransition.h"
#include "transformTransition.h"
#include "transformAttribute.h"

#include <boundingSphere.h>
#include <nodeAttributes.h>
#include <nodeTransitionWrapper.h>
#include <allTransitionsWrapper.h>
#include <allAttributesWrapper.h>
#include <graphicsStateGuardian.h>
#include <renderTraverser.h>
#include <geomSphere.h>

TypeHandle DrawBoundsTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DrawBoundsTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DrawBoundsTransition::
DrawBoundsTransition() {
  RenderModeAttribute *rma = new RenderModeAttribute;
  rma->set_mode(RenderModeProperty::M_wireframe);
  CullFaceAttribute *cfao = new CullFaceAttribute;
  cfao->set_mode(CullFaceProperty::M_cull_clockwise);
  ColorAttribute *cao = new ColorAttribute;
  cao->set_on(0.3, 1.0, 0.5, 1.0);

  CullFaceAttribute *cfai = new CullFaceAttribute;
  cfai->set_mode(CullFaceProperty::M_cull_counter_clockwise);
  ColorAttribute *cai = new ColorAttribute;
  cai->set_on(0.15, 0.5, 0.25, 1.0);

  _outside_attrib.set_attribute(RenderModeTransition::get_class_type(), rma);
  _outside_attrib.set_attribute(CullFaceTransition::get_class_type(), cfao);
  _outside_attrib.set_attribute(ColorTransition::get_class_type(), cao);

  _inside_attrib.set_attribute(CullFaceTransition::get_class_type(), cfai);
  _inside_attrib.set_attribute(ColorTransition::get_class_type(), cai);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawBoundsTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DrawBoundsTransition just
//               like this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DrawBoundsTransition::
make_copy() const {
  return new DrawBoundsTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawBoundsTransition::sub_render
//       Access: Public, Virtual
//  Description: This is called by the RenderTraverser to tell the
//               drawBounds to do its thing.
////////////////////////////////////////////////////////////////////
bool DrawBoundsTransition::
sub_render(NodeRelation *arc, const AllAttributesWrapper &attrib,
           AllTransitionsWrapper &, RenderTraverser *trav) {
  GraphicsStateGuardian *gsg = trav->get_gsg();

  const BoundingVolume &vol = arc->get_bound();
  if (!vol.is_empty() && !vol.is_infinite()) {
    // This is a bit of work, because the bounding volume has already
    // been transformed by this arc's transform.

    LMatrix4f mat;
    TransformAttribute *ta;
    if (get_attribute_into(ta, attrib, TransformTransition::get_class_type())) {
      mat = ta->get_matrix();
    } else {
      mat = LMatrix4f::ident_mat();
    }

    TransformTransition *tt;
    if (get_transition_into(tt, arc)) {
      // Undo the application of the arc's transform.  Yuck.
      mat = ::invert(tt->get_matrix()) * mat;
    }

    TransformAttribute *new_ta = new TransformAttribute;
    new_ta->set_matrix(mat);
    _outside_attrib.set_attribute(TransformTransition::get_class_type(), new_ta);

    gsg->set_state(_outside_attrib, true);

    if (vol.is_of_type(BoundingSphere::get_class_type())) {
      const BoundingSphere *sphere = DCAST(BoundingSphere, &vol);

      GeomSphere geom;
      PTA_Vertexf verts;
      LPoint3f center = sphere->get_center();
      verts.push_back(center);
      center[0] += sphere->get_radius();
      verts.push_back(center);
      geom.set_coords(verts, G_PER_VERTEX);
      geom.set_num_prims(1);

      gsg->draw_sphere(&geom);
      gsg->set_state(_inside_attrib, false);
      gsg->draw_sphere(&geom);

    } else {
      sgattrib_cat.warning()
        << "Don't know how to draw a representation of "
        << vol.get_class_type() << "\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawBoundsTransition::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool DrawBoundsTransition::
has_sub_render() const {
  return true;
}

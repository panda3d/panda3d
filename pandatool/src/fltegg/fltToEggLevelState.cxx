/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltToEggLevelState.cxx
 * @author drose
 * @date 2001-04-18
 */

#include "fltToEggLevelState.h"
#include "fltToEggConverter.h"
#include "fltTransformTranslate.h"
#include "fltTransformRotateAboutPoint.h"
#include "fltTransformRotateAboutEdge.h"
#include "fltTransformScale.h"
#include "fltTransformPut.h"
#include "eggGroup.h"
#include "dcast.h"
#include "look_at.h"


/**
 *
 */
FltToEggLevelState::
~FltToEggLevelState() {
  Parents::iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    delete (*pi).second;
  }
}

/**
 *
 */
FltToEggLevelState::ParentNodes::
ParentNodes() {
  _axial_billboard = nullptr;
  _point_billboard = nullptr;
  _plain = nullptr;
}

/**
 * Sometimes it is necessary to synthesize a group within a particular
 * EggGroup, for instance to insert a transform or billboard flag.  This
 * function will synthesize a group as needed, or return an existing group (if
 * the group need not be synthesized, or if a matching group was previously
 * synthesized).
 *
 * This collects together polygons that share the same billboard axis and/or
 * transform space into the same group, rather than wastefully creating a
 * group per polygon.
 */
EggGroupNode *FltToEggLevelState::
get_synthetic_group(const std::string &name,
                    const FltBead *transform_bead,
                    FltGeometry::BillboardType type) {
  LMatrix4d transform = transform_bead->get_transform();
  bool is_identity = transform.almost_equal(LMatrix4d::ident_mat());
  if (is_identity &&
      (type != FltGeometry::BT_axial &&
       type != FltGeometry::BT_point)) {
    // Trivial case: the primitive belongs directly in its parent group node.
    return _egg_parent;
  }

  // For other cases, we may have to create a subgroup to put the primitive
  // into.
  Parents::iterator pi;
  pi = _parents.find(transform);
  ParentNodes *nodes;
  if (pi != _parents.end()) {
    nodes = (*pi).second;
  } else {
    nodes = new ParentNodes;
    _parents.insert(Parents::value_type(transform, nodes));
  }

  switch (type) {
  case FltGeometry::BT_axial:
    if (nodes->_axial_billboard == nullptr) {
      nodes->_axial_billboard = new EggGroup(name);
      _egg_parent->add_child(nodes->_axial_billboard);
      nodes->_axial_billboard->set_billboard_type(EggGroup::BT_axis);
      if (!is_identity) {
        set_transform(transform_bead, nodes->_axial_billboard);
        nodes->_axial_billboard->set_group_type(EggGroup::GT_instance);
      }
    }
    return nodes->_axial_billboard;

  case FltGeometry::BT_point:
    if (nodes->_point_billboard == nullptr) {
      nodes->_point_billboard = new EggGroup(name);
      _egg_parent->add_child(nodes->_point_billboard);
      nodes->_point_billboard->set_billboard_type(EggGroup::BT_point_world_relative);
      if (!is_identity) {
        set_transform(transform_bead, nodes->_point_billboard);
        nodes->_point_billboard->set_group_type(EggGroup::GT_instance);
      }
    }
    return nodes->_point_billboard;

  default: // Normally, BT_none, although we've occasionally seen a
           // value of 3 come in here, whatever that's supposed to mean.
    if (nodes->_plain == nullptr) {
      nodes->_plain = new EggGroup(name);
      _egg_parent->add_child(nodes->_plain);
      if (!is_identity) {
        set_transform(transform_bead, nodes->_plain);
        nodes->_plain->set_group_type(EggGroup::GT_instance);
      }
    }
    return nodes->_plain;
  }
}

/**
 * Sets up the group to reflect the transform indicated by the given record,
 * if any.
 */
void FltToEggLevelState::
set_transform(const FltBead *flt_bead, EggGroup *egg_group) {
  if (flt_bead->has_transform()) {
    egg_group->set_group_type(EggGroup::GT_instance);

    int num_steps = flt_bead->get_num_transform_steps();
    bool componentwise_ok = !_converter->_compose_transforms;

    if (num_steps == 0) {
      componentwise_ok = false;
    } else {
      // Walk through each transform step and store the individual components
      // in the egg file.  If we come across a step we don't know how to
      // interpret, just store the whole transform matrix in the egg file.
      egg_group->clear_transform();
      for (int i = num_steps -1; i >= 0 && componentwise_ok; i--) {
        const FltTransformRecord *step = flt_bead->get_transform_step(i);
        if (step->is_exact_type(FltTransformTranslate::get_class_type())) {
          const FltTransformTranslate *trans;
          DCAST_INTO_V(trans, step);
          if (!trans->get_delta().almost_equal(LVector3d::zero())) {
            egg_group->add_translate3d(trans->get_delta());
          }

        } else if (step->is_exact_type(FltTransformRotateAboutPoint::get_class_type())) {
          const FltTransformRotateAboutPoint *rap;
          DCAST_INTO_V(rap, step);
          if (!IS_NEARLY_ZERO(rap->get_angle())) {
            if (!rap->get_center().almost_equal(LVector3d::zero())) {
              egg_group->add_translate3d(-rap->get_center());
            }
            LVector3d axis = LCAST(double, rap->get_axis());
            egg_group->add_rotate3d(rap->get_angle(), axis);
            if (!rap->get_center().almost_equal(LVector3d::zero())) {
              egg_group->add_translate3d(rap->get_center());
            }
          }

        } else if (step->is_exact_type(FltTransformRotateAboutEdge::get_class_type())) {
          const FltTransformRotateAboutEdge *rae;
          DCAST_INTO_V(rae, step);
          if (!IS_NEARLY_ZERO(rae->get_angle())) {
            if (!rae->get_point_a().almost_equal(LVector3d::zero())) {
              egg_group->add_translate3d(-rae->get_point_a());
            }
            LVector3d axis = rae->get_point_b() - rae->get_point_a();
            egg_group->add_rotate3d(rae->get_angle(), axis);
            if (!rae->get_point_a().almost_equal(LVector3d::zero())) {
              egg_group->add_translate3d(rae->get_point_a());
            }
          }

        } else if (step->is_exact_type(FltTransformScale::get_class_type())) {
          const FltTransformScale *scale;
          DCAST_INTO_V(scale, step);
          if (!scale->get_scale().almost_equal(LVecBase3(1.0f, 1.0f, 1.0f))) {
            if (scale->has_center() &&
                !scale->get_center().almost_equal(LVector3d::zero())) {
              egg_group->add_translate3d(-scale->get_center());
            }
            egg_group->add_scale3d(LCAST(double, scale->get_scale()));
            if (scale->has_center() &&
                !scale->get_center().almost_equal(LVector3d::zero())) {
              egg_group->add_translate3d(scale->get_center());
            }
          }

        } else if (step->is_exact_type(FltTransformPut::get_class_type())) {
          const FltTransformPut *put;
          DCAST_INTO_V(put, step);

          if (!put->get_from_origin().almost_equal(LVector3d::zero())) {
            egg_group->add_translate3d(-put->get_from_origin());
          }
          LQuaterniond q1, q2;
          look_at(q1, put->get_from_align() - put->get_from_origin(),
                  put->get_from_track() - put->get_from_origin(),
                  CS_zup_right);
          look_at(q2, put->get_to_align() - put->get_to_origin(),
                  put->get_to_track() - put->get_to_origin(),
                  CS_zup_right);

          LQuaterniond q = invert(q1) * q2;

          if (!q.is_identity()) {
            egg_group->add_rotate3d(q);
          }
          if (!put->get_to_origin().almost_equal(LVector3d::zero())) {
            egg_group->add_translate3d(put->get_to_origin());
          }

        } else {
          // Here's a transform component we haven't implemented here.  Give
          // up on storing the componentwise transform.
          componentwise_ok = false;
        }
      }
    }

    if (!componentwise_ok) {
      egg_group->set_transform3d(flt_bead->get_transform());
    }
  }
}

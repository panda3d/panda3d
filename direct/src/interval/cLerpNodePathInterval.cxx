// Filename: cLerpNodePathInterval.cxx
// Created by:  drose (27Aug02)
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

#include "cLerpNodePathInterval.h"
#include "lerp_helpers.h"
#include "transformState.h"
#include "renderState.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "dcast.h"
#include "config_interval.h"

TypeHandle CLerpNodePathInterval::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::Constructor
//       Access: Published
//  Description: Constructs a lerp interval that will lerp some
//               properties on the indicated node, possibly relative
//               to the indicated other node (if other is nonempty).
//
//               You must call set_end_pos(), etc. for the various
//               properties you wish to lerp before the first call to
//               priv_initialize().  If you want to set a starting value
//               for any of the properties, you may call
//               set_start_pos(), etc.; otherwise, the starting value
//               is taken from the actual node's value at the time the
//               lerp is performed.
//
//               The starting values may be explicitly specified or
//               omitted.  The value of bake_in_start determines the
//               behavior if the starting values are omitted.  If
//               bake_in_start is true, the values are obtained the
//               first time the lerp runs, and thenceforth are stored
//               within the interval.  If bake_in_start is false, the
//               starting value is computed each frame, based on
//               assuming the current value represents the value set
//               from the last time the interval was run.  This
//               "smart" behavior allows code to manipulate the object
//               event while it is being lerped, and the lerp
//               continues to apply in a sensible way.
////////////////////////////////////////////////////////////////////
CLerpNodePathInterval::
CLerpNodePathInterval(const string &name, double duration, 
                      CLerpInterval::BlendType blend_type,
                      bool bake_in_start,
                      const NodePath &node, const NodePath &other) :
  CLerpInterval(name, duration, blend_type),
  _node(node),
  _other(other),
  _flags(0)
{
  if (bake_in_start) {
    _flags |= F_bake_in_start;
  }
  _prev_d = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::initialize
//       Access: Published, Virtual
//  Description: This replaces the first call to priv_step(), and indicates
//               that the interval has just begun.  This may be
//               overridden by derived classes that need to do some
//               explicit initialization on the first call.
////////////////////////////////////////////////////////////////////
void CLerpNodePathInterval::
priv_initialize(double t) {
  check_stopped(get_class_type(), "priv_initialize");
  recompute();
  _prev_d = 0.0;
  _state = S_started;
  priv_step(t);
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::instant
//       Access: Published, Virtual
//  Description: This is called in lieu of priv_initialize() .. priv_step()
//               .. priv_finalize(), when everything is to happen within
//               one frame.  The interval should initialize itself,
//               then leave itself in the final state.
////////////////////////////////////////////////////////////////////
void CLerpNodePathInterval::
priv_instant() {
  check_stopped(get_class_type(), "priv_instant");
  recompute();
  _prev_d = 0.0;
  _state = S_started;
  priv_step(get_duration());
  _state = S_final;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::step
//       Access: Published, Virtual
//  Description: Advances the time on the interval.  The time may
//               either increase (the normal case) or decrease
//               (e.g. if the interval is being played by a slider).
////////////////////////////////////////////////////////////////////
void CLerpNodePathInterval::
priv_step(double t) {
  check_started(get_class_type(), "priv_step");
  _state = S_started;
  double d = compute_delta(t);

  if ((_flags & (F_end_pos | F_end_hpr | F_end_scale)) != 0) {
    // We have some transform lerp.
    CPT(TransformState) transform;

    if (_other.is_empty()) {
      // If there is no other node, it's a local transform lerp.
      transform = _node.get_transform();
    } else {
      // If there *is* another node, we get the transform relative to
      // that node.
      transform = _node.get_transform(_other);
    }
    
    LPoint3f pos;
    LVecBase3f hpr;
    LVecBase3f scale;

    if ((_flags & F_end_pos) != 0) {
      if ((_flags & F_start_pos) != 0) {
        lerp_value(pos, d, _start_pos, _end_pos);

      } else if ((_flags & F_bake_in_start) != 0) {
        // Get the current starting pos, and bake it in.
        set_start_pos(transform->get_pos());
        lerp_value(pos, d, _start_pos, _end_pos);

      } else {
        // "smart" lerp from the current pos to the new pos.
        pos = transform->get_pos();
        lerp_value_from_prev(pos, d, _prev_d, pos, _end_pos);
      }
    }
    if ((_flags & F_end_hpr) != 0) {
      if ((_flags & F_start_hpr) != 0) {
        lerp_value(hpr, d, _start_hpr, _end_hpr);

      } else if ((_flags & F_bake_in_start) != 0) {
        set_start_hpr(transform->get_hpr());
        lerp_value(hpr, d, _start_hpr, _end_hpr);

      } else {
        hpr = transform->get_hpr();
        lerp_value_from_prev(hpr, d, _prev_d, hpr, _end_hpr);
      }
    }
    if ((_flags & F_end_scale) != 0) {
      if ((_flags & F_start_scale) != 0) {
        lerp_value(scale, d, _start_scale, _end_scale);

      } else if ((_flags & F_bake_in_start) != 0) {
        set_start_scale(transform->get_scale());
        lerp_value(scale, d, _start_scale, _end_scale);

      } else {
        scale = transform->get_scale();
        lerp_value_from_prev(scale, d, _prev_d, scale, _end_scale);
      }
    }

    // Now apply the modifications back to the transform.  We want to
    // be a little careful here, because we don't want to assume the
    // transform has hpr/scale components if they're not needed.  And
    // in any case, we only want to apply the components that we
    // computed, above.
    switch (_flags & (F_end_pos | F_end_hpr | F_end_scale)) {
    case F_end_pos:
      if (_other.is_empty()) {
        _node.set_pos(pos);
      } else {
        _node.set_pos(_other, pos);
      }
      break;

    case F_end_hpr:
      if (_other.is_empty()) {
        _node.set_hpr(hpr);
      } else {
        _node.set_hpr(_other, hpr);
      }
      break;

    case F_end_scale:
      if (_other.is_empty()) {
        _node.set_scale(scale);
      } else {
        _node.set_scale(_other, scale);
      }
      break;

    case F_end_hpr | F_end_scale:
      if (_other.is_empty()) {
        _node.set_hpr_scale(hpr, scale);
      } else {
        _node.set_hpr_scale(hpr, scale);
      }
      break;

    case F_end_pos | F_end_hpr:
      if (_other.is_empty()) {
        _node.set_pos_hpr(pos, hpr);
      } else {
        _node.set_pos_hpr(_other, pos, hpr);
      }
      break;

    case F_end_pos | F_end_scale:
      if (transform->quat_given()) {
        if (_other.is_empty()) {
          _node.set_pos_quat_scale(pos, transform->get_quat(), scale);
        } else {
          _node.set_pos_quat_scale(_other, pos, transform->get_quat(), scale);
        }
      } else {
        if (_other.is_empty()) {
          _node.set_pos_hpr_scale(pos, transform->get_hpr(), scale);
        } else {
          _node.set_pos_hpr_scale(_other, pos, transform->get_hpr(), scale);
        }
      }
      break;

    case F_end_pos | F_end_hpr | F_end_scale:
      if (_other.is_empty()) {
        _node.set_pos_hpr_scale(pos, hpr, scale);
      } else {
        _node.set_pos_hpr_scale(_other, pos, hpr, scale);
      }
      break;

    default:
      interval_cat.error()
        << "Internal error in CLerpNodePathInterval::priv_step().\n";
    }
  }

  if ((_flags & (F_end_color | F_end_color_scale)) != 0) {
    // We have some render state lerp.
    CPT(RenderState) state;

    if (_other.is_empty()) {
      // If there is no other node, it's a local state lerp.  This is
      // most common.
      state = _node.get_state();
    } else {
      // If there *is* another node, we get the state relative to that
      // node.  This is weird, but you could lerp color (for instance)
      // relative to some other node's color.
      state = _node.get_state(_other);
    }
    
    // Unlike in the transform case above, we can go ahead and modify
    // the state immediately with each attribute change, since these
    // attributes don't interrelate.

    if ((_flags & F_end_color) != 0) {
      Colorf color;

      if ((_flags & F_start_color) != 0) {
        lerp_value(color, d, _start_color, _end_color);

      } else {
        // Get the previous color.
        color.set(1.0f, 1.0f, 1.0f, 1.0f);
        const RenderAttrib *attrib =
          state->get_attrib(ColorAttrib::get_class_type());
        if (attrib != (const RenderAttrib *)NULL) {
          const ColorAttrib *ca = DCAST(ColorAttrib, attrib);
          if (ca->get_color_type() == ColorAttrib::T_flat) {
            color = ca->get_color();
          }
        }

        lerp_value_from_prev(color, d, _prev_d, color, _end_color);
      }

      state = state->add_attrib(ColorAttrib::make_flat(color));
    }

    if ((_flags & F_end_color_scale) != 0) {
      LVecBase4f color_scale;

      if ((_flags & F_start_color_scale) != 0) {
        lerp_value(color_scale, d, _start_color_scale, _end_color_scale);

      } else {
        // Get the previous color scale.
        color_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
        const RenderAttrib *attrib =
          state->get_attrib(ColorScaleAttrib::get_class_type());
        if (attrib != (const RenderAttrib *)NULL) {
          const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, attrib);
          color_scale = csa->get_scale();
        }

        lerp_value_from_prev(color_scale, d, _prev_d, color_scale, _end_color_scale);
      }

      state = state->add_attrib(ColorScaleAttrib::make(color_scale));
    }    

    // Now apply the new state back to the node.
    if (_other.is_empty()) {
      _node.set_state(state);
    } else {
      _node.set_state(_other, state);
    }
  }

  _prev_d = d;
  _curr_t = t;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::reverse_initialize
//       Access: Published, Virtual
//  Description: Similar to priv_initialize(), but this is called when the
//               interval is being played backwards; it indicates that
//               the interval should start at the finishing state and
//               undo any intervening intervals.
////////////////////////////////////////////////////////////////////
void CLerpNodePathInterval::
priv_reverse_initialize(double t) {
  check_stopped(get_class_type(), "priv_reverse_initialize");
  recompute();
  _state = S_started;
  _prev_d = 1.0;
  priv_step(t);
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::reverse_instant
//       Access: Published, Virtual
//  Description: This is called in lieu of priv_reverse_initialize()
//               .. priv_step() .. priv_reverse_finalize(), when everything is
//               to happen within one frame.  The interval should
//               initialize itself, then leave itself in the initial
//               state.
////////////////////////////////////////////////////////////////////
void CLerpNodePathInterval::
priv_reverse_instant() {
  check_stopped(get_class_type(), "priv_reverse_initialize");
  recompute();
  _state = S_started;
  _prev_d = 1.0;
  priv_step(0.0);
  _state = S_initial;
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpNodePathInterval::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CLerpNodePathInterval::
output(ostream &out) const {
  out << get_name() << ":";

  if ((_flags & F_end_pos) != 0) {
    out << " pos";
    if ((_flags & F_start_pos) != 0) {
      out << " from " << _start_pos;
    }
    out << " to " << _end_pos;
  }

  if ((_flags & F_end_hpr) != 0) {
    out << " hpr";
    if ((_flags & F_start_hpr) != 0) {
      out << " from " << _start_hpr;
    }
    out << " to " << _end_hpr;
  }

  if ((_flags & F_end_scale) != 0) {
    out << " scale";
    if ((_flags & F_start_scale) != 0) {
      out << " from " << _start_scale;
    }
    out << " to " << _end_scale;
  }

  if ((_flags & F_end_color) != 0) {
    out << " color";
    if ((_flags & F_start_color) != 0) {
      out << " from " << _start_color;
    }
    out << " to " << _end_color;
  }

  if ((_flags & F_end_color_scale) != 0) {
    out << " color_scale";
    if ((_flags & F_start_color_scale) != 0) {
      out << " from " << _start_color_scale;
    }
    out << " to " << _end_color_scale;
  }

  out << " dur " << get_duration();
}

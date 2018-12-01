/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cConstrainPosInterval.cxx
 * @author pratt
 * @date 2006-09-29
 */

#include "cConstrainPosInterval.h"
#include "config_interval.h"
#include "lvecBase3.h"

TypeHandle CConstrainPosInterval::_type_handle;

/**
 * Constructs a constraint interval that will constrain the position of one
 * node to the position of another.
 *
 * If wrt is true, the node's position will be transformed into the target
 * node's parent's  space before being copied.  If wrt is false, the target
 * node's local position will be copied unaltered.
 */
CConstrainPosInterval::
CConstrainPosInterval(const std::string &name, double duration,
                      const NodePath &node, const NodePath &target,
                      bool wrt, const LVecBase3 posOffset) :
  CConstraintInterval(name, duration),
  _node(node),
  _target(target),
  _wrt(wrt),
  _posOffset(posOffset)
{
}

/**
 * Advances the time on the interval.  The time may either increase (the
 * normal case) or decrease (e.g.  if the interval is being played by a
 * slider).
 */
void CConstrainPosInterval::
priv_step(double t) {
  check_started(get_class_type(), "priv_step");
  _state = S_started;
  _curr_t = t;

  if(! _target.is_empty()) {
    if(_wrt) {
      if(! _node.is_same_graph(_target)){
        interval_cat.warning()
          << "Unable to copy position in CConstrainPosInterval::priv_step;\n"
          << "node (" << _node.get_name()
          << ") and target (" << _target.get_name()
          << ") are not in the same graph.\n";
        return;
      }
      _target.set_pos(_node, _posOffset);
    } else {
      if(_posOffset == LVector3::zero()) {
        _target.set_pos(_node.get_pos());
      } else {
        _target.set_pos(_node.get_pos() + _posOffset);
      }
    }
  }
}

/**
 *
 */
void CConstrainPosInterval::
output(std::ostream &out) const {
  out << get_name() << ":";
  out << " dur " << get_duration();
}

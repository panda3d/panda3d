/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cConstrainPosHprInterval.cxx
 * @author pratt
 * @date 2008-03-10
 */

#include "cConstrainPosHprInterval.h"
#include "config_interval.h"
#include "lvecBase3.h"

TypeHandle CConstrainPosHprInterval::_type_handle;

/**
 * Constructs a constraint interval that will constrain the position and
 * orientation of one node to the position and orientation of another.
 *
 * If wrt is true, the node's position and orientation will be transformed
 * into the target node's parent's space before being copied.  If wrt is
 * false, the target node's local position and orientation will be copied
 * unaltered.
 */
CConstrainPosHprInterval::
CConstrainPosHprInterval(const std::string &name, double duration,
                         const NodePath &node, const NodePath &target,
                         bool wrt, const LVecBase3 posOffset,
                         const LVecBase3 hprOffset) :
  CConstraintInterval(name, duration),
  _node(node),
  _target(target),
  _wrt(wrt),
  _posOffset(posOffset)
{
  _quatOffset.set_hpr(hprOffset);
}

/**
 * Advances the time on the interval.  The time may either increase (the
 * normal case) or decrease (e.g.  if the interval is being played by a
 * slider).
 */
void CConstrainPosHprInterval::
priv_step(double t) {
  check_started(get_class_type(), "priv_step");
  _state = S_started;
  _curr_t = t;

  if(! _target.is_empty()) {
    if(_wrt) {
      if(! _node.is_same_graph(_target)){
        interval_cat.warning()
          << "Unable to copy position and orientation in CConstrainPosHprInterval::priv_step;\n"
          << "node (" << _node.get_name()
          << ") and target (" << _target.get_name()
          << ") are not in the same graph.\n";
        return;
      }
      _target.set_pos_quat(_node, _posOffset, _quatOffset);
    } else {
      _target.set_pos_quat(_node.get_pos() + _posOffset, _quatOffset*_node.get_quat());
    }
  }
}

/**
 *
 */
void CConstrainPosHprInterval::
output(std::ostream &out) const {
  out << get_name() << ":";
  out << " dur " << get_duration();
}

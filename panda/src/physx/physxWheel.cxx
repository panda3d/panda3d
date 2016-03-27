/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxWheel.cxx
 * @author enn0x
 * @date 2010-03-23
 */

#include "physxWheel.h"
#include "physxWheelDesc.h"
#include "physxWheelShape.h"

TypeHandle PhysxWheel::_type_handle;

/**
 *
 */
/*
PhysxWheelShape *PhysxWheel::
get_wheel_shape() const {

  return _wheelShape;
}
*/

/**
 * Attaches a node path to this wheel.  The node path's transform will be
 * updated automatically.
 *
 * Note: any non-uniform scale or shear set on the NodePath's transform will
 * be overwritten at the time of the first update.
 */
/*
void PhysxWheel::
attach_node_path(const NodePath &np) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!np.is_empty());
  _np = NodePath(np);
}
*/

/**
 * Detaches a previously assigned NodePath from this wheel.  The NodePath's
 * transform will no longer be updated.
 */
/*
void PhysxWheel::
detach_node_path() {

  nassertv(_error_type == ET_ok);
  _np = NodePath();
}
*/

/**
 * Retrieves a previously attached NodePath.  An empty NodePath will be
 * returned if no NodePath has been attached to this wheel.
 */
/*
NodePath PhysxWheel::
get_node_path() const {

  nassertr(_error_type == ET_ok, NodePath::fail());
  return _np;
}
*/

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSpace_ext.I
 * @author rdb
 * @date 2013-12-11
 */

/* okcircular */
#include "odeGeom_ext.h"

/**
 * A more Pythonic way of calling getAABB()
 */
INLINE PyObject *Extension<OdeSpace>::
get_AA_bounds() const {
  LPoint3f *min_point = new LPoint3f;
  LPoint3f *max_point = new LPoint3f;
  _this->get_AABB(*min_point, *max_point);

  PyObject *min_inst = DTool_CreatePyInstance(min_point, true);
  PyObject *max_inst = DTool_CreatePyInstance(max_point, true);
  return Py_BuildValue("NN", min_inst, max_inst);
}

/**
 * Equivalent to get_geom(index).convert()
 */
INLINE PyObject *Extension<OdeSpace>::
get_converted_geom(int index) const {
  OdeGeom g = _this->get_geom(index);
  return invoke_extension(&g).convert();
}

/**
 * Equivalent to get_space().convert()
 */
INLINE PyObject *Extension<OdeSpace>::
get_converted_space() const {
  OdeSpace s = _this->get_space();
  return invoke_extension(&s).convert();
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerEvent_ext.cxx
 * @author rdb
 * @date 2020-12-31
 */

#include "collisionHandlerEvent_ext.h"
#include "collisionHandlerFloor.h"
#include "collisionHandlerGravity.h"
#include "collisionHandlerPusher.h"

#ifdef HAVE_PYTHON

/**
 * Implements pickling behavior.
 */
PyObject *Extension<CollisionHandlerEvent>::
__reduce__(PyObject *self) const {
  Datagram dg;
  _this->write_datagram(dg);

  const char *data = (const char *)dg.get_data();
  Py_ssize_t size = dg.get_length();
  return Py_BuildValue("O()y#", Py_TYPE(self), data, size);
}

/**
 * Takes the value returned by __getstate__ and uses it to freshly initialize
 * this CollisionHandlerEvent object.
 */
void Extension<CollisionHandlerEvent>::
__setstate__(PyObject *self, vector_uchar data) {
  Datagram dg(std::move(data));
  DatagramIterator scan(dg);
  _this->read_datagram(scan);
}

#endif

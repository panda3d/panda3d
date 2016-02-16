/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dNoneObject.h
 * @author drose
 * @date 2009-06-30
 */

#ifndef P3DNONEOBJECT_H
#define P3DNONEOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

/**
 * An object type that contains no value, similar to Python's None type, or
 * JavaScript's null type.
 */
class P3DNoneObject : public P3DObject {
public:
  P3DNoneObject();

public:
  virtual P3D_object_type get_type();
  virtual bool get_bool();
  virtual void make_string(string &value);
};

#endif

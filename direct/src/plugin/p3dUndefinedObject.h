/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dUndefinedObject.h
 * @author drose
 * @date 2009-07-07
 */

#ifndef P3DUNDEFINEDOBJECT_H
#define P3DUNDEFINEDOBJECT_H

#include "p3d_plugin_common.h"
#include "p3dObject.h"

/**
 * An object type that represents an undefined value.  Python doesn't have
 * such a concept, but JavaScript does, and it is sometimes an important
 * return value.
 */
class P3DUndefinedObject : public P3DObject {
public:
  P3DUndefinedObject();

public:
  virtual P3D_object_type get_type();
  virtual bool get_bool();
  virtual void make_string(std::string &value);
};

#endif

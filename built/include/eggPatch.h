/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPatch.h
 * @author drose
 * @date 2012-04-27
 */

#ifndef EGGPATCH_H
#define EGGPATCH_H

#include "pandabase.h"

#include "eggPrimitive.h"

/**
 * A single "patch", a special primitive to be rendered only with a
 * tessellation shader.
 */
class EXPCL_PANDA_EGG EggPatch : public EggPrimitive {
PUBLISHED:
  INLINE explicit EggPatch(const std::string &name = "");
  INLINE EggPatch(const EggPatch &copy);
  INLINE EggPatch &operator = (const EggPatch &copy);

  virtual EggPatch *make_copy() const override;

  virtual void write(std::ostream &out, int indent_level) const override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggPatch",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

};

#include "eggPatch.I"

#endif

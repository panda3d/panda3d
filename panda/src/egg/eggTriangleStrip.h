/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTriangleStrip.h
 * @author drose
 * @date 2005-03-13
 */

#ifndef EGGTRIANGLESTRIP_H
#define EGGTRIANGLESTRIP_H

#include "pandabase.h"

#include "eggCompositePrimitive.h"

/**
 * A connected strip of triangles.  This does not normally appear in an egg
 * file; it is typically generated as a result of meshing.
 */
class EXPCL_PANDA_EGG EggTriangleStrip : public EggCompositePrimitive {
PUBLISHED:
  INLINE explicit EggTriangleStrip(const std::string &name = "");
  INLINE EggTriangleStrip(const EggTriangleStrip &copy);
  INLINE EggTriangleStrip &operator = (const EggTriangleStrip &copy);
  virtual ~EggTriangleStrip();

  virtual EggTriangleStrip *make_copy() const override;

  virtual void write(std::ostream &out, int indent_level) const override;

protected:
  virtual int get_num_lead_vertices() const override;
  virtual bool do_triangulate(EggGroupNode *container) const override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggCompositePrimitive::init_type();
    register_type(_type_handle, "EggTriangleStrip",
                  EggCompositePrimitive::get_class_type());
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

#include "eggTriangleStrip.I"

#endif

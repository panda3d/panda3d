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
class EXPCL_PANDAEGG EggTriangleStrip : public EggCompositePrimitive {
PUBLISHED:
  INLINE EggTriangleStrip(const string &name = "");
  INLINE EggTriangleStrip(const EggTriangleStrip &copy);
  INLINE EggTriangleStrip &operator = (const EggTriangleStrip &copy);
  virtual ~EggTriangleStrip();

  virtual EggTriangleStrip *make_copy() const OVERRIDE;

  virtual void write(ostream &out, int indent_level) const OVERRIDE;

protected:
  virtual int get_num_lead_vertices() const OVERRIDE;
  virtual bool do_triangulate(EggGroupNode *container) const OVERRIDE;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggCompositePrimitive::init_type();
    register_type(_type_handle, "EggTriangleStrip",
                  EggCompositePrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const OVERRIDE {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() OVERRIDE {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "eggTriangleStrip.I"

#endif

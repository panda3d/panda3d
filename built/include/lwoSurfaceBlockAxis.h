/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlockAxis.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACEBLOCKAXIS_H
#define LWOSURFACEBLOCKAXIS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

/**
 * Indicates the axis for this particular shader's projection.  This works in
 * conjunction with LwoSurfaceBlockProjection.  This is a subchunk of
 * LwoSurfaceBlock.
 */
class LwoSurfaceBlockAxis : public LwoChunk {
public:
  enum Axis {
    A_x        = 0,
    A_y        = 1,
    A_z        = 2
  };
  Axis _axis;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoChunk::init_type();
    register_type(_type_handle, "LwoSurfaceBlockAxis",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

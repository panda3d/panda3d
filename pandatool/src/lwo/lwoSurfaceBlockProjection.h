/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlockProjection.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACEBLOCKPROJECTION_H
#define LWOSURFACEBLOCKPROJECTION_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

/**
 * Indicates the projection mode for this particular shader.  This determines
 * how UV coordinates should be computed based on the vertex positions.  This
 * is a subchunk of LwoSurfaceBlock.
 */
class LwoSurfaceBlockProjection : public LwoChunk {
public:
  enum Mode {
    M_planar        = 0,
    M_cylindrical   = 1,
    M_spherical     = 2,
    M_cubic         = 3,
    M_front         = 4,
    M_uv            = 5
  };
  Mode _mode;

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
    register_type(_type_handle, "LwoSurfaceBlockProjection",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlockWrap.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACEBLOCKWRAP_H
#define LWOSURFACEBLOCKWRAP_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

/**
 * Specifies how the texture image appears for areas outside the image.
 */
class LwoSurfaceBlockWrap : public LwoChunk {
public:
  enum Mode {
    M_reset    = 0,    // black outside
    M_repeat   = 1,    // standard repeat
    M_mirror   = 2,    // repeat with reflection
    M_edge     = 3     // GL-style clamping
  };
  Mode _width, _height;

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
    register_type(_type_handle, "LwoSurfaceBlockWrap",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

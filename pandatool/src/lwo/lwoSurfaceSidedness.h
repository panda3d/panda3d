/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceSidedness.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACESIDEDNESS_H
#define LWOSURFACESIDEDNESS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

/**
 * Records whether polygons are frontfacing only or backfacing also.  This is
 * associated with the LwoSurface chunk.
 */
class LwoSurfaceSidedness : public LwoChunk {
public:
  enum Sidedness {
    S_front          = 1,
    S_front_and_back = 3
  };

  Sidedness _sidedness;

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
    register_type(_type_handle, "LwoSurfaceSidedness",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

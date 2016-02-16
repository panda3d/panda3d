/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoBoundingBox.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOBOUNDINGBOX_H
#define LWOBOUNDINGBOX_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "luse.h"

/**
 * Stores the bounding box for the vertex data in a layer.  Optional.
 */
class LwoBoundingBox : public LwoChunk {
public:
  LVecBase3 _min;
  LVecBase3 _max;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

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
    register_type(_type_handle, "LwoBoundingBox",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

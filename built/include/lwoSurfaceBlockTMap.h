/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlockTMap.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACEBLOCKTMAP_H
#define LWOSURFACEBLOCKTMAP_H

#include "pandatoolbase.h"

#include "lwoGroupChunk.h"

/**
 * The tMap chunk within a LwoSurfaceBlock chunk.
 */
class LwoSurfaceBlockTMap : public LwoGroupChunk {
public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

  virtual IffChunk *make_new_chunk(IffInputFile *in, IffId id);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LwoGroupChunk::init_type();
    register_type(_type_handle, "LwoSurfaceBlockTMap",
                  LwoGroupChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurface.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACE_H
#define LWOSURFACE_H

#include "pandatoolbase.h"

#include "lwoGroupChunk.h"

/**
 * Describes the shading attributes of a surface.  This is similar to the
 * concept usually called a "material" or "shader" in other file formats.
 */
class LwoSurface : public LwoGroupChunk {
public:
  std::string _name;
  std::string _source;

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
    register_type(_type_handle, "LwoSurface",
                  LwoGroupChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

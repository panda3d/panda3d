/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoTags.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOTAGS_H
#define LWOTAGS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "luse.h"
#include "vector_string.h"

/**
 * An array of tag strings that will be referenced by later chunks.
 *
 * This also serves as an array of surface names to be referenced by a later
 * LwoPolygons chunk, in 5.x LWOB files.  The chunk id can be used to
 * differentiate the meaning (TAGS vs.  SRFS).
 */
class LwoTags : public LwoChunk {
public:
  int get_num_tags() const;
  std::string get_tag(int n) const;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef vector_string Tags;
  Tags _tags;

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
    register_type(_type_handle, "LwoTags",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

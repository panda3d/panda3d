/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoPoints.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOPOINTS_H
#define LWOPOINTS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "luse.h"

/**
 * An array of points that will be referenced by later chunks.
 */
class LwoPoints : public LwoChunk {
public:
  int get_num_points() const;
  const LPoint3 &get_point(int n) const;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef pvector<LPoint3> Points;
  Points _points;

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
    register_type(_type_handle, "LwoPoints",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

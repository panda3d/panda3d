/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoPolygonTags.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOPOLYGONTAGS_H
#define LWOPOLYGONTAGS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

/**
 * An association of polygons defined in the most recent LwoPolygons chunk to
 * tag ids defined in the most recent LwoTags chunk.  This associated
 * properties with the polygons, depending on the tag_type.
 */
class LwoPolygonTags : public LwoChunk {
public:
  bool has_tag(int polygon_index) const;
  int get_tag(int polygon_index) const;

  IffId _tag_type;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef pmap<int, int> TMap;
  TMap _tmap;

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
    register_type(_type_handle, "LwoPolygonTags",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

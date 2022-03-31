/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoPolygons.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOPOLYGONS_H
#define LWOPOLYGONS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "luse.h"
#include "vector_int.h"
#include "referenceCount.h"
#include "pointerTo.h"

/**
 * An array of polygons that will be referenced by later chunks.
 */
class LwoPolygons : public LwoChunk {
public:
  enum PolygonFlags {
    PF_continuity_1    = 0x0400,
    PF_continuity_2    = 0x0800,
    PF_numverts_mask   = 0x03f,

    // This "flag" is stored artificially when reading 5.x LWOB files, and
    // indicates that the polygon is a decal of a preceding polygon.
    PF_decal           = 0x0001
  };

  class Polygon : public ReferenceCount {
  public:
    int _flags;
    vector_int _vertices;

    // This value is only filled in when reading 5.x LWOB files, and indicates
    // the surface index of the polygon within a preceding SRFS (LwoTags)
    // chunk.  For 6.x and later files, this will be set to -1.
    int _surface_index;
  };

  int get_num_polygons() const;
  Polygon *get_polygon(int n) const;

  IffId _polygon_type;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef pvector< PT(Polygon) > Polygons;
  Polygons _polygons;

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
    register_type(_type_handle, "LwoPolygons",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

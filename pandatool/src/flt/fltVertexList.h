/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltVertexList.h
 * @author drose
 * @date 2000-08-25
 */

#ifndef FLTVERTEXLIST_H
#define FLTVERTEXLIST_H

#include "pandatoolbase.h"

#include "fltRecord.h"
#include "fltPackedColor.h"
#include "fltVertex.h"

#include "pointerTo.h"

/**
 * A list of vertices, typically added as a child of a face bead.
 */
class FltVertexList : public FltRecord {
public:
  FltVertexList(FltHeader *header);

  int get_num_vertices() const;
  FltVertex *get_vertex(int n) const;
  void clear_vertices();
  void add_vertex(FltVertex *vertex);

  virtual void output(std::ostream &out) const;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

private:
  typedef pvector<PT(FltVertex)> Vertices;
  Vertices _vertices;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltVertexList",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

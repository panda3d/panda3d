// Filename: lwoTags.h
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef LWOTAGS_H
#define LWOTAGS_H

#include "pandatoolbase.h"

#include "lwoChunk.h"

#include "luse.h"
#include "vector_string.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoTags
// Description : An array of tag strings that will be referenced by
//               later chunks.
//
//               This also serves as an array of surface names to be
//               referenced by a later LwoPolygons chunk, in 5.x LWOB
//               files.  The chunk id can be used to differentiate the
//               meaning (TAGS vs. SRFS).
////////////////////////////////////////////////////////////////////
class LwoTags : public LwoChunk {
public:
  int get_num_tags() const;
  string get_tag(int n) const;

public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

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



// Filename: lwoGroupChunk.h
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

#ifndef LWOGROUPCHUNK_H
#define LWOGROUPCHUNK_H

#include "pandatoolbase.h"

#include "lwoChunk.h"
#include "iffChunk.h"

#include "pointerTo.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoGroupChunk
// Description : A particular kind of LwoChunk that is expected to
//               contain an arbitrary number of child chunks.
////////////////////////////////////////////////////////////////////
class LwoGroupChunk : public LwoChunk {
public:
  int get_num_chunks() const;
  IffChunk *get_chunk(int n) const;

protected:
  bool read_chunks_iff(IffInputFile *in, size_t stop_at);
  bool read_subchunks_iff(IffInputFile *in, size_t stop_at);
  void write_chunks(ostream &out, int indent_level) const;

  typedef pvector< PT(IffChunk) > Chunks;
  Chunks _chunks;

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
    register_type(_type_handle, "LwoGroupChunk",
                  LwoChunk::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif



// Filename: lwoSurfaceBlock.h
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef LWOSURFACEBLOCK_H
#define LWOSURFACEBLOCK_H

#include "pandatoolbase.h"

#include "lwoGroupChunk.h"
#include "lwoSurfaceBlockHeader.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoSurfaceBlock
// Description : A texture layer or shader, part of a LwoSurface
//               chunk.
////////////////////////////////////////////////////////////////////
class LwoSurfaceBlock : public LwoGroupChunk {
public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual IffChunk *make_new_chunk(IffInputFile *in, IffId id);

  PT(LwoSurfaceBlockHeader) _header;

private:
  static TypeHandle _type_handle;
};

#endif





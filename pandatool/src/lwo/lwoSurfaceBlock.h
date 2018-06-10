/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlock.h
 * @author drose
 * @date 2001-04-24
 */

#ifndef LWOSURFACEBLOCK_H
#define LWOSURFACEBLOCK_H

#include "pandatoolbase.h"

#include "lwoGroupChunk.h"
#include "lwoSurfaceBlockHeader.h"

/**
 * A texture layer or shader, part of a LwoSurface chunk.
 */
class LwoSurfaceBlock : public LwoGroupChunk {
public:
  virtual bool read_iff(IffInputFile *in, size_t stop_at);
  virtual void write(std::ostream &out, int indent_level = 0) const;

  virtual IffChunk *make_new_chunk(IffInputFile *in, IffId id);

  PT(LwoSurfaceBlockHeader) _header;

private:
  static TypeHandle _type_handle;
};

#endif

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataBlock.h
 * @author drose
 * @date 2007-06-04
 */

#ifndef VERTEXDATABLOCK_H
#define VERTEXDATABLOCK_H

#include "pandabase.h"
#include "simpleAllocator.h"
#include "vertexDataPage.h"
#include "referenceCount.h"

class VertexDataPage;
class VertexDataBlock;

/**
 * A block of bytes that stores the actual raw vertex data referenced by a
 * GeomVertexArrayData object.
 */
class EXPCL_PANDA_GOBJ VertexDataBlock : public SimpleAllocatorBlock, public ReferenceCount {
protected:
  INLINE VertexDataBlock(VertexDataPage *page,
                         size_t start, size_t size);

PUBLISHED:
  INLINE VertexDataPage *get_page() const;
  INLINE VertexDataBlock *get_next_block() const;

public:
  INLINE unsigned char *get_pointer(bool force) const;

  friend class VertexDataPage;
};

#include "vertexDataBlock.I"

#endif

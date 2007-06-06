// Filename: vertexDataBlock.h
// Created by:  drose (04Jun07)
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

#ifndef VERTEXDATABLOCK_H
#define VERTEXDATABLOCK_H

#include "pandabase.h"
#include "simpleAllocator.h"
#include "referenceCount.h"

class VertexDataPage;
class VertexDataBlock;

////////////////////////////////////////////////////////////////////
//       Class : VertexDataBlock
// Description : A block of bytes that stores the actual raw vertex
//               data referenced by a GeomVertexArrayData object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataBlock : public SimpleAllocatorBlock, public ReferenceCount {
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

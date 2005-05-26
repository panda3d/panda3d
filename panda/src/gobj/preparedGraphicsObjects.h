// Filename: preparedGraphicsObjects.h
// Created by:  drose (19Feb04)
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

#ifndef PREPAREDGRAPHICSOBJECTS_H
#define PREPAREDGRAPHICSOBJECTS_H

#include "pandabase.h"
#include "referenceCount.h"
#include "texture.h"
#include "geom.h"
#include "qpgeomVertexArrayData.h"
#include "qpgeomPrimitive.h"
#include "pointerTo.h"
#include "pStatCollector.h"
#include "pset.h"
#include "pmutex.h"

class TextureContext;
class GeomContext;
class VertexBufferContext;
class IndexBufferContext;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : PreparedGraphicsObjects
// Description : A table of objects that are saved within the graphics
//               context for reference by handle later.  Generally,
//               this represents things like OpenGL texture objects or
//               display lists (or their equivalent on other
//               platforms).
//
//               This object simply records the pointers to the
//               context objects created by the individual GSG's;
//               these context objects will contain enough information
//               to reference or release the actual object stored
//               within the graphics context.
//
//               These tables may potentially be shared between
//               related graphics contexts, hence their storage here
//               in a separate object rather than as a part of the
//               GraphicsStateGuardian.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PreparedGraphicsObjects : public ReferenceCount {
public:
  PreparedGraphicsObjects();
  ~PreparedGraphicsObjects();

  INLINE void release_all();

  void enqueue_texture(Texture *tex);
  bool dequeue_texture(Texture *tex);
  void release_texture(TextureContext *tc);
  int release_all_textures();

  TextureContext *prepare_texture_now(Texture *tex, GraphicsStateGuardianBase *gsg);

  void enqueue_geom(Geom *geom);
  bool dequeue_geom(Geom *geom);
  void release_geom(GeomContext *gc);
  int release_all_geoms();

  GeomContext *prepare_geom_now(Geom *geom, GraphicsStateGuardianBase *gsg);

  void enqueue_vertex_buffer(qpGeomVertexArrayData *data);
  bool dequeue_vertex_buffer(qpGeomVertexArrayData *data);
  void release_vertex_buffer(VertexBufferContext *vbc);
  int release_all_vertex_buffers();

  VertexBufferContext *
  prepare_vertex_buffer_now(qpGeomVertexArrayData *data,
                            GraphicsStateGuardianBase *gsg);

  void enqueue_index_buffer(qpGeomPrimitive *data);
  bool dequeue_index_buffer(qpGeomPrimitive *data);
  void release_index_buffer(IndexBufferContext *ibc);
  int release_all_index_buffers();

  IndexBufferContext *
  prepare_index_buffer_now(qpGeomPrimitive *data,
                           GraphicsStateGuardianBase *gsg);

  void update(GraphicsStateGuardianBase *gsg);

private:
  typedef phash_set<TextureContext *, pointer_hash> Textures;
  typedef phash_set< PT(Texture) > EnqueuedTextures;
  typedef phash_set<GeomContext *, pointer_hash> Geoms;
  typedef phash_set< PT(Geom) > EnqueuedGeoms;
  typedef phash_set<VertexBufferContext *, pointer_hash> VertexBuffers;
  typedef phash_set< PT(qpGeomVertexArrayData) > EnqueuedVertexBuffers;
  typedef phash_set<IndexBufferContext *, pointer_hash> IndexBuffers;
  typedef phash_set< PT(qpGeomPrimitive) > EnqueuedIndexBuffers;

  Mutex _lock;
  Textures _prepared_textures, _released_textures;  
  EnqueuedTextures _enqueued_textures;
  Geoms _prepared_geoms, _released_geoms;  
  EnqueuedGeoms _enqueued_geoms;
  VertexBuffers _prepared_vertex_buffers, _released_vertex_buffers;  
  EnqueuedVertexBuffers _enqueued_vertex_buffers;
  IndexBuffers _prepared_index_buffers, _released_index_buffers;  
  EnqueuedIndexBuffers _enqueued_index_buffers;

  static PStatCollector _total_texusage_pcollector;
  static PStatCollector _total_buffers_pcollector;

  friend class GraphicsStateGuardian;
};

#include "preparedGraphicsObjects.I"

#endif

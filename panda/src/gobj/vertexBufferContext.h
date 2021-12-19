/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexBufferContext.h
 * @author drose
 * @date 2005-03-17
 */

#ifndef VERTEXBUFFERCONTEXT_H
#define VERTEXBUFFERCONTEXT_H

#include "pandabase.h"

#include "bufferContext.h"
#include "geomVertexArrayData.h"
#include "preparedGraphicsObjects.h"
#include "adaptiveLru.h"

/**
 * This is a special class object that holds all the information returned by a
 * particular GSG to indicate the vertex data array's internal context
 * identifier.
 *
 * This allows the GSG to cache the vertex data array in whatever way makes
 * sense.  For instance, DirectX can allocate a vertex buffer for the array.
 * OpenGL can create a buffer object.
 */
class EXPCL_PANDA_GOBJ VertexBufferContext : public BufferContext, public AdaptiveLruPage {
public:
  INLINE VertexBufferContext(PreparedGraphicsObjects *pgo,
                             GeomVertexArrayData *data);

PUBLISHED:
  INLINE GeomVertexArrayData *get_data() const;

  INLINE bool changed_size(const GeomVertexArrayDataHandle *reader) const;
  INLINE bool changed_usage_hint(const GeomVertexArrayDataHandle *reader) const;
  INLINE bool was_modified(const GeomVertexArrayDataHandle *reader) const;

public:
  INLINE void update_data_size_bytes(size_t new_data_size_bytes);
  INLINE void mark_loaded(const GeomVertexArrayDataHandle *reader);
  INLINE void mark_unloaded();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

private:
  GeomEnums::UsageHint _usage_hint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BufferContext::init_type();
    register_type(_type_handle, "VertexBufferContext",
                  BufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

inline std::ostream &operator << (std::ostream &out, const VertexBufferContext &context) {
  context.output(out);
  return out;
}

#include "vertexBufferContext.I"

#endif

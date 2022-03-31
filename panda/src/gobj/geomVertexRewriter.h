/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexRewriter.h
 * @author drose
 * @date 2005-03-28
 */

#ifndef GEOMVERTEXREWRITER_H
#define GEOMVERTEXREWRITER_H

#include "pandabase.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"

/**
 * This object provides the functionality of both a GeomVertexReader and a
 * GeomVertexWriter, combined together into one convenient package.  It is
 * designed for making a single pass over a GeomVertexData object, modifying
 * rows as it goes.
 *
 * Although it doesn't provide any real performance benefit over using a
 * separate reader and writer on the same data, it should probably be used in
 * preference to a separate reader and writer, because it makes an effort to
 * manage the reference counts properly between the reader and the writer to
 * avoid accidentally dereferencing either array while recopying.
 */
class EXPCL_PANDA_GOBJ GeomVertexRewriter : public GeomVertexWriter, public GeomVertexReader {
PUBLISHED:
  INLINE GeomVertexRewriter(Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexRewriter(GeomVertexData *vertex_data, Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexRewriter(GeomVertexData *vertex_data,
                            CPT_InternalName name,
                            Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexRewriter(GeomVertexArrayData *array_data, Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexRewriter(GeomVertexArrayData *array_data,
                            int column, Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexRewriter(const GeomVertexRewriter &copy);
  INLINE void operator = (const GeomVertexRewriter &copy);
  INLINE ~GeomVertexRewriter();

  INLINE GeomVertexData *get_vertex_data() const;
  INLINE GeomVertexArrayData *get_array_data() const;
  INLINE GeomVertexArrayDataHandle *get_array_handle() const;
  INLINE size_t get_stride() const;
  INLINE Thread *get_current_thread() const;

  INLINE bool set_column(int column);
  INLINE bool set_column(CPT_InternalName name);
  INLINE bool set_column(int array, const GeomVertexColumn *column);

  INLINE void clear();
  INLINE bool has_column() const;
  INLINE int get_array() const;
  INLINE const GeomVertexColumn *get_column() const;

  INLINE void set_row_unsafe(int row);
  INLINE void set_row(int row);

  INLINE int get_start_row() const;
  INLINE bool is_at_end() const;

  void output(std::ostream &out) const;
};

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexRewriter &rewriter) {
  rewriter.output(out);
  return out;
}

#include "geomVertexRewriter.I"

#endif

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexWriter.h
 * @author drose
 * @date 2005-03-25
 */

#ifndef GEOMVERTEXWRITER_H
#define GEOMVERTEXWRITER_H

#include "pandabase.h"
#include "geomVertexData.h"
#include "geomVertexColumn.h"
#include "internalName.h"
#include "luse.h"
#include "pointerTo.h"

/**
 * This object provides a high-level interface for quickly writing a sequence
 * of numeric values from a vertex table.
 *
 * This object can be used both to replace existing vertices in the table, or
 * to extend the table with new vertices.  The set_data*() family of methods
 * can only be used to replace existing data; it is an error to allow these to
 * run past the end of the data.  The add_data*() family of methods, on the
 * other hand, can be used to replace existing data or add new data; if you
 * call set_row() into the middle of existing data the add_data*() methods
 * will behave like the corresponding set_data*(), but if they run past the
 * end of existing data they will quietly add new vertices.
 *
 * Like GeomVertexReader, the writer is particularly optimized for writing a
 * single column of data values for a series of vertices, without changing
 * columns between each number.  Although you can also use one
 * GeomVertexWriter to write across the columns if it is convenient, by
 * calling set_column() repeatedly at each vertex, it is faster to write down
 * the columns, and to use a different GeomVertexWriter for each column.
 *
 * Note that, like a GeomVertexReader, a GeomVertexWriter does not keep a
 * reference count to the actual vertex data buffer.  This means that it is
 * important not to keep a GeomVertexWriter object around over a long period
 * of time in which the data buffer is likely to be deallocated; it is
 * intended for making a quick pass over the data in one session.
 *
 * It also means that you should create any GeomVertexWriters *before*
 * creating GeomVertexReaders on the same data, since the writer itself might
 * cause the vertex buffer to be deallocated.  Better yet, use a
 * GeomVertexRewriter if you are going to create both of them anyway.
 */
class EXPCL_PANDA_GOBJ GeomVertexWriter : public GeomEnums {
PUBLISHED:
  INLINE GeomVertexWriter(Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexWriter(GeomVertexData *vertex_data,
                          Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexWriter(GeomVertexData *vertex_data,
                          CPT_InternalName name,
                          Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexWriter(GeomVertexArrayData *array_data,
                          Thread *current_thread = Thread::get_current_thread());
  INLINE GeomVertexWriter(GeomVertexArrayData *array_data,
                          int column,
                          Thread *current_thread = Thread::get_current_thread());

public:
  INLINE GeomVertexWriter(GeomVertexDataPipelineWriter *data_writer,
                          const InternalName *name);

PUBLISHED:
  INLINE GeomVertexWriter(const GeomVertexWriter &copy);
  INLINE void operator = (const GeomVertexWriter &copy);
  INLINE ~GeomVertexWriter();

  INLINE GeomVertexData *get_vertex_data() const;
  INLINE GeomVertexArrayData *get_array_data() const;
  INLINE GeomVertexArrayDataHandle *get_array_handle() const;
  INLINE size_t get_stride() const;
  INLINE Thread *get_current_thread() const;

  INLINE bool set_column(int column);
  INLINE bool set_column(CPT_InternalName name);
  bool set_column(int array, const GeomVertexColumn *column);
  INLINE void clear();
  bool reserve_num_rows(int num_rows);

  INLINE bool has_column() const;
  INLINE int get_array() const;
  INLINE const GeomVertexColumn *get_column() const;

  INLINE void set_row_unsafe(int row);
  INLINE void set_row(int row);

  INLINE int get_start_row() const;
  INLINE int get_write_row() const;
  INLINE bool is_at_end() const;

  INLINE void set_data1f(float data);
  INLINE void set_data2f(float x, float y);
  INLINE void set_data2f(const LVecBase2f &data);
  INLINE void set_data3f(float x, float y, float z);
  INLINE void set_data3f(const LVecBase3f &data);
  INLINE void set_data4f(float x, float y, float z, float w);
  INLINE void set_data4f(const LVecBase4f &data);
  INLINE void set_matrix3f(const LMatrix3f &mat);
  INLINE void set_matrix4f(const LMatrix4f &mat);

  INLINE void set_data1d(double data);
  INLINE void set_data2d(double x, double y);
  INLINE void set_data2d(const LVecBase2d &data);
  INLINE void set_data3d(double x, double y, double z);
  INLINE void set_data3d(const LVecBase3d &data);
  INLINE void set_data4d(double x, double y, double z, double w);
  INLINE void set_data4d(const LVecBase4d &data);
  INLINE void set_matrix3d(const LMatrix3d &mat);
  INLINE void set_matrix4d(const LMatrix4d &mat);

  INLINE void set_data1(PN_stdfloat data);
  INLINE void set_data2(PN_stdfloat x, PN_stdfloat y);
  INLINE void set_data2(const LVecBase2 &data);
  INLINE void set_data3(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE void set_data3(const LVecBase3 &data);
  INLINE void set_data4(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat w);
  INLINE void set_data4(const LVecBase4 &data);
  INLINE void set_matrix3(const LMatrix3 &mat);
  INLINE void set_matrix4(const LMatrix4 &mat);

  INLINE void set_data1i(int data);
  INLINE void set_data2i(int a, int b);
  INLINE void set_data2i(const LVecBase2i &data);
  INLINE void set_data3i(int a, int b, int c);
  INLINE void set_data3i(const LVecBase3i &data);
  INLINE void set_data4i(int a, int b, int c, int d);
  INLINE void set_data4i(const LVecBase4i &data);

  INLINE void add_data1f(float data);
  INLINE void add_data2f(float x, float y);
  INLINE void add_data2f(const LVecBase2f &data);
  INLINE void add_data3f(float x, float y, float z);
  INLINE void add_data3f(const LVecBase3f &data);
  INLINE void add_data4f(float x, float y, float z, float w);
  INLINE void add_data4f(const LVecBase4f &data);
  INLINE void add_matrix3f(const LMatrix3f &mat);
  INLINE void add_matrix4f(const LMatrix4f &mat);

  INLINE void add_data1d(double data);
  INLINE void add_data2d(double x, double y);
  INLINE void add_data2d(const LVecBase2d &data);
  INLINE void add_data3d(double x, double y, double z);
  INLINE void add_data3d(const LVecBase3d &data);
  INLINE void add_data4d(double x, double y, double z, double w);
  INLINE void add_data4d(const LVecBase4d &data);
  INLINE void add_matrix3d(const LMatrix3d &mat);
  INLINE void add_matrix4d(const LMatrix4d &mat);

  INLINE void add_data1(PN_stdfloat data);
  INLINE void add_data2(PN_stdfloat x, PN_stdfloat y);
  INLINE void add_data2(const LVecBase2 &data);
  INLINE void add_data3(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE void add_data3(const LVecBase3 &data);
  INLINE void add_data4(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat w);
  INLINE void add_data4(const LVecBase4 &data);
  INLINE void add_matrix3(const LMatrix3 &mat);
  INLINE void add_matrix4(const LMatrix4 &mat);

  INLINE void add_data1i(int data);
  INLINE void add_data2i(int a, int b);
  INLINE void add_data2i(const LVecBase2i &data);
  INLINE void add_data3i(int a, int b, int c);
  INLINE void add_data3i(const LVecBase3i &data);
  INLINE void add_data4i(int a, int b, int c, int d);
  INLINE void add_data4i(const LVecBase4i &data);

  void output(std::ostream &out) const;

protected:
  INLINE GeomVertexColumn::Packer *get_packer() const;

private:
  class Writer;

  void initialize();

  INLINE void set_pointer(int row);
  INLINE void quick_set_pointer(int row);
  INLINE unsigned char *inc_pointer();
  INLINE unsigned char *inc_add_pointer();

  bool set_vertex_column(int array, const GeomVertexColumn *column,
                         GeomVertexDataPipelineWriter *data_writer);
  bool set_array_column(const GeomVertexColumn *column);

  // It is important that we only store *one* of the following two pointers.
  // If we are storing a GeomVertexDataarray index, we must not keep a pointer
  // to the particular ArrayData we are working on (if we do, it may result in
  // an extra copy of the data due to holding the reference count).
  PT(GeomVertexData) _vertex_data;
  int _array;
  PT(GeomVertexArrayData) _array_data;

  Thread *_current_thread;
  GeomVertexColumn::Packer *_packer;
  int _stride;

  PT(GeomVertexArrayDataHandle) _handle;
  unsigned char *_pointer_begin;
  unsigned char *_pointer_end;
  unsigned char *_pointer;

  int _start_row;

#ifdef _DEBUG
  // This is defined just for the benefit of having something non-NULL to
  // return from a nassertr() call.
  static unsigned char empty_buffer[100];
#endif
};

INLINE std::ostream &
operator << (std::ostream &out, const GeomVertexWriter &writer) {
  writer.output(out);
  return out;
}

#include "geomVertexWriter.I"

#endif

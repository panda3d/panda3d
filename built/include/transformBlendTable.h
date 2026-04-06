/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformBlendTable.h
 * @author drose
 * @date 2005-03-24
 */

#ifndef TRANSFORMBLENDTABLE_H
#define TRANSFORMBLENDTABLE_H

#include "pandabase.h"
#include "transformBlend.h"
#include "vertexTransform.h"
#include "copyOnWriteObject.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"
#include "indirectLess.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "sparseArray.h"

class FactoryParams;

/**
 * This structure collects together the different combinations of transforms
 * and blend amounts used by a GeomVertexData, to facilitate computing dynamic
 * vertices on the CPU at runtime.  Each vertex has a pointer to exactly one
 * of the entries in this table, and each entry defines a number of
 * transform/blend combinations.
 *
 * This structure is used for a GeomVertexData set up to compute its dynamic
 * vertices on the CPU.  See TransformTable for one set up to compute its
 * dynamic vertices on the graphics card.
 */
class EXPCL_PANDA_GOBJ TransformBlendTable : public CopyOnWriteObject {
protected:
  virtual PT(CopyOnWriteObject) make_cow_copy();

PUBLISHED:
  TransformBlendTable();
  TransformBlendTable(const TransformBlendTable &copy);
  void operator = (const TransformBlendTable &copy);
  virtual ~TransformBlendTable();

  INLINE size_t get_num_blends() const;
  INLINE const TransformBlend &get_blend(size_t n) const;
  MAKE_SEQ(get_blends, get_num_blends, get_blend);
  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;

  void set_blend(size_t n, const TransformBlend &blend);
  void remove_blend(size_t n);
  size_t add_blend(const TransformBlend &blend);

  INLINE int get_num_transforms() const;
  INLINE int get_max_simultaneous_transforms() const;

  INLINE void set_rows(const SparseArray &rows);
  INLINE const SparseArray &get_rows() const;
  INLINE SparseArray &modify_rows();

  void write(std::ostream &out, int indent_level) const;

  MAKE_SEQ_PROPERTY(blends, get_num_blends, get_blend, set_blend, remove_blend);
  MAKE_PROPERTY(modified, get_modified);
  MAKE_PROPERTY(num_transforms, get_num_transforms);
  MAKE_PROPERTY(max_simultaneous_transforms, get_max_simultaneous_transforms);
  MAKE_PROPERTY(rows, get_rows, set_rows);

private:
  class CData;

  void clear_index();
  INLINE void consider_rebuild_index() const;
  void rebuild_index();

  void recompute_modified(CData *cdata, Thread *current_thread);
  void clear_modified(Thread *current_thread);

private:
  // We don't bother with registering the table, or protecting its data in a
  // CycleData structure--the interface on GeomVertexData guarantees that the
  // pointer will be copied if we modify the table.
  typedef pvector<TransformBlend> Blends;
  Blends _blends;

  SparseArray _rows;

  // This map indexes directly into the above vector.  That means any time we
  // add or remove anything from the vector, we must completely rebuild the
  // index (since the vector might reallocate, invalidating all the pointers
  // into it).
  typedef pmap<const TransformBlend *, int, IndirectLess<TransformBlend> > BlendIndex;
  BlendIndex _blend_index;
  int _num_transforms;
  int _max_simultaneous_transforms;

  // Even though we don't store the actual blend table data in a CycleData
  // structure, we do need to keep a local cache of the relevant modified
  // stamps there, so it can be updated per-thread.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return TransformBlendTable::get_class_type();
    }

    UpdateSeq _modified;
    UpdateSeq _global_modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "TransformBlendTable",
                  CopyOnWriteObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class VertexTransform;
};

INLINE std::ostream &operator << (std::ostream &out, const TransformBlendTable &obj);

#include "transformBlendTable.I"

#endif

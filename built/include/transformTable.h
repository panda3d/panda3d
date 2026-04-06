/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformTable.h
 * @author drose
 * @date 2005-03-23
 */

#ifndef TRANSFORMTABLE_H
#define TRANSFORMTABLE_H

#include "pandabase.h"
#include "vertexTransform.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "luse.h"
#include "pvector.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class FactoryParams;

/**
 * Stores the total set of VertexTransforms that the vertices in a particular
 * GeomVertexData object might depend on.
 *
 * This structure is used for a GeomVertexData set up to compute its dynamic
 * vertices on the graphics card.  See TransformBlendTable for one set up to
 * compute its dynamic vertices on the CPU.
 */
class EXPCL_PANDA_GOBJ TransformTable : public TypedWritableReferenceCount {
PUBLISHED:
  TransformTable();
  TransformTable(const TransformTable &copy);
  void operator = (const TransformTable &copy);
  virtual ~TransformTable();

  INLINE bool is_registered() const;
  INLINE static CPT(TransformTable) register_table(const TransformTable *table);

  INLINE size_t get_num_transforms() const;
  INLINE const VertexTransform *get_transform(size_t n) const;
  MAKE_SEQ(get_transforms, get_num_transforms, get_transform);
  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;

  void set_transform(size_t n, const VertexTransform *transform);
  void insert_transform(size_t n, const VertexTransform *transform);
  void remove_transform(size_t n);
  size_t add_transform(const VertexTransform *transform);

  void write(std::ostream &out) const;

  MAKE_PROPERTY(registered, is_registered);
  MAKE_PROPERTY(modified, get_modified);
  MAKE_SEQ_PROPERTY(transforms, get_num_transforms, get_transform, set_transform,
                                remove_transform, insert_transform);

private:
  void do_register();
  void do_unregister();
  INLINE void update_modified(UpdateSeq modified, Thread *current_thread);

private:
  bool _is_registered;

  typedef pvector< CPT(VertexTransform) > Transforms;
  Transforms _transforms;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return TransformTable::get_class_type();
    }

    UpdateSeq _modified;
  };

  PipelineCycler<CData> _cycler;
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
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "TransformTable",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class VertexTransform;
};

INLINE std::ostream &operator << (std::ostream &out, const TransformTable &obj);

#include "transformTable.I"

#endif

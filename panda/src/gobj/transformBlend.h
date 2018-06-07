/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformBlend.h
 * @author drose
 * @date 2005-03-24
 */

#ifndef TRANSFORMBLEND_H
#define TRANSFORMBLEND_H

#include "pandabase.h"
#include "vertexTransform.h"
#include "pointerTo.h"
#include "pvector.h"
#include "ordered_vector.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

/**
 * This defines a single entry in a TransformBlendTable.  It represents a
 * unique combination of VertexTransform pointers and blend amounts.
 */
class EXPCL_PANDA_GOBJ TransformBlend {
PUBLISHED:
  INLINE TransformBlend();
  INLINE TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0);
  INLINE TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0,
                        const VertexTransform *transform1, PN_stdfloat weight1);
  INLINE TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0,
                        const VertexTransform *transform1, PN_stdfloat weight1,
                        const VertexTransform *transform2, PN_stdfloat weight2);
  INLINE TransformBlend(const VertexTransform *transform0, PN_stdfloat weight0,
                        const VertexTransform *transform1, PN_stdfloat weight1,
                        const VertexTransform *transform2, PN_stdfloat weight2,
                        const VertexTransform *transform3, PN_stdfloat weight3);
  INLINE TransformBlend(const TransformBlend &copy);
  INLINE void operator = (const TransformBlend &copy);
  INLINE ~TransformBlend();

  int compare_to(const TransformBlend &other) const;
  INLINE bool operator < (const TransformBlend &other) const;
  INLINE bool operator == (const TransformBlend &other) const;
  INLINE bool operator != (const TransformBlend &other) const;

  void add_transform(const VertexTransform *transform, PN_stdfloat weight);
  void remove_transform(const VertexTransform *transform);
  void limit_transforms(int max_transforms);
  void normalize_weights();
  bool has_transform(const VertexTransform *transform) const;
  PN_stdfloat get_weight(const VertexTransform *transform) const;

  INLINE size_t get_num_transforms() const;
  INLINE const VertexTransform *get_transform(size_t n) const;
  MAKE_SEQ(get_transforms, get_num_transforms, get_transform);
  INLINE PN_stdfloat get_weight(size_t n) const;
  INLINE void remove_transform(size_t n);
  INLINE void set_transform(size_t n, const VertexTransform *transform);
  INLINE void set_weight(size_t n, PN_stdfloat weight);

  MAKE_SEQ_PROPERTY(transforms, get_num_transforms, get_transform,
                    set_transform, remove_transform);
  MAKE_MAP_PROPERTY(weights, has_transform, get_weight);
  MAKE_MAP_KEYS_SEQ(weights, get_num_transforms, get_transform);

  INLINE void update_blend(Thread *current_thread) const;

  INLINE void get_blend(LMatrix4 &result, Thread *current_thread) const;

  INLINE void transform_point(LPoint4f &point, Thread *current_thread) const;
  INLINE void transform_point(LPoint3f &point, Thread *current_thread) const;
  INLINE void transform_vector(LVector3f &point, Thread *current_thread) const;

  INLINE void transform_point(LPoint4d &point, Thread *current_thread) const;
  INLINE void transform_point(LPoint3d &point, Thread *current_thread) const;
  INLINE void transform_vector(LVector3d &point, Thread *current_thread) const;

  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(modified, get_modified);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

private:
  class CData;

  void recompute_result(CData *cdata, Thread *current_thread);
  void clear_result(Thread *current_thread);

  class TransformEntry {
  public:
    INLINE bool operator < (const TransformEntry &other) const;

    CPT(VertexTransform) _transform;
    PN_stdfloat _weight;
  };
  typedef ov_set<TransformEntry> Entries;
  Entries _entries;

  // This is the data that must be cycled between pipeline stages; it is just
  // a local cache.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return TransformBlend::get_class_type();
    }

    LMatrix4 _result;
    UpdateSeq _modified;
    UpdateSeq _global_modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  void write_datagram(BamWriter *manager, Datagram &dg) const;
  int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

  friend class VertexTransform;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "TransformBlend");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const TransformBlend &obj);

#include "transformBlend.I"

#endif

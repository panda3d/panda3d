// Filename: transformBlend.h
// Created by:  drose (24Mar05)
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

#ifndef TRANSFORMBLEND_H
#define TRANSFORMBLEND_H

#include "pandabase.h"
#include "vertexTransform.h"
#include "pointerTo.h"
#include "pvector.h"
#include "ordered_vector.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

////////////////////////////////////////////////////////////////////
//       Class : TransformBlend
// Description : This defines a single entry in a
//               TransformBlendTable.  It represents a unique
//               combination of VertexTransform pointers and blend
//               amounts.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformBlend {
PUBLISHED:
  INLINE TransformBlend();
  INLINE TransformBlend(const VertexTransform *transform0, float weight0);
  INLINE TransformBlend(const VertexTransform *transform0, float weight0,
                        const VertexTransform *transform1, float weight1);
  INLINE TransformBlend(const VertexTransform *transform0, float weight0,
                        const VertexTransform *transform1, float weight1,
                        const VertexTransform *transform2, float weight2);
  INLINE TransformBlend(const VertexTransform *transform0, float weight0,
                        const VertexTransform *transform1, float weight1,
                        const VertexTransform *transform2, float weight2,
                        const VertexTransform *transform3, float weight3);
  INLINE TransformBlend(const TransformBlend &copy);
  INLINE void operator = (const TransformBlend &copy);
  INLINE ~TransformBlend();

  int compare_to(const TransformBlend &other) const;
  INLINE bool operator < (const TransformBlend &other) const;
  INLINE bool operator == (const TransformBlend &other) const;
  INLINE bool operator != (const TransformBlend &other) const;

  void add_transform(const VertexTransform *transform, float weight);
  void remove_transform(const VertexTransform *transform);
  void normalize_weights();
  bool has_transform(const VertexTransform *transform) const;
  float get_weight(const VertexTransform *transform) const;

  INLINE int get_num_transforms() const;
  INLINE const VertexTransform *get_transform(int n) const;
  INLINE float get_weight(int n) const;
  INLINE void set_transform(int n, const VertexTransform *transform);
  INLINE void set_weight(int n, float weight);

  INLINE void update_blend() const;

  INLINE void get_blend(LMatrix4f &result) const;
  INLINE void transform_point(LPoint4f &point) const;
  INLINE void transform_point(LPoint3f &point) const;
  INLINE void transform_vector(LVector3f &point) const;
  INLINE UpdateSeq get_modified() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

private:
  class TransformEntry {
  public:
    INLINE bool operator < (const TransformEntry &other) const;

    CPT(VertexTransform) _transform;
    float _weight;
  };
  typedef ov_set<TransformEntry> Entries;
  Entries _entries;

  // This is the data that must be cycled between pipeline stages; it
  // is just a local cache.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;

    LMatrix4f _result;
    UpdateSeq _modified;
    UpdateSeq _global_modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void recompute_result(CDWriter &cdata);
  void clear_result();

public:
  void write_datagram(BamWriter *manager, Datagram &dg) const;
  int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

  friend class VertexTransform;
};

INLINE ostream &operator << (ostream &out, const TransformBlend &obj);

#include "transformBlend.I"

#endif

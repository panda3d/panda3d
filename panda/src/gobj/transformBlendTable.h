// Filename: transformBlendTable.h
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

#ifndef TRANSFORMBLENDTABLE_H
#define TRANSFORMBLENDTABLE_H

#include "pandabase.h"
#include "transformBlend.h"
#include "vertexTransform.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"
#include "indirectLess.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

////////////////////////////////////////////////////////////////////
//       Class : TransformBlendTable
// Description : This structure collects together the different
//               combinations of transforms and blend amounts used by
//               a GeomVertexData, to facilitate computing dynamic
//               vertices on the CPU at runtime.  Each vertex has a
//               pointer to exactly one of entries in this table,
//               and each entry defines a number of transform/blend
//               combinations.
//
//               This structure is used for a GeomVertexData set up to
//               compute its dynamic vertices on the CPU.  See
//               TransformTable for one set up to compute its
//               dynamic vertices on the graphics card.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformBlendTable : public TypedWritableReferenceCount {
PUBLISHED:
  TransformBlendTable();
  TransformBlendTable(const TransformBlendTable &copy);
  void operator = (const TransformBlendTable &copy);
  virtual ~TransformBlendTable();

  INLINE int get_num_blends() const;
  INLINE const TransformBlend &get_blend(int n) const;
  INLINE UpdateSeq get_modified() const;

  void set_blend(int n, const TransformBlend &blend);
  void remove_blend(int n);
  int add_blend(const TransformBlend &blend);

  INLINE int get_num_transforms() const;
  INLINE int get_max_simultaneous_transforms() const;

  void write(ostream &out, int indent_level) const;

private:
  void clear_index();
  INLINE void consider_rebuild_index() const;
  void rebuild_index();

private:
  // We don't bother with registering the table, or protecting its
  // data in a CycleData structure--the interface on GeomVertexData
  // guarantees that the pointer will be copied if we modify the
  // table.
  typedef pvector<TransformBlend> Blends;
  Blends _blends;

  // This map indexes directly into the above vector.  That means any
  // time we add or remove anything from the vector, we must
  // completely rebuild the index (since the vector might reallocate,
  // invalidating all the pointers into it).
  typedef pmap<const TransformBlend *, int, IndirectLess<TransformBlend> > BlendIndex;
  BlendIndex _blend_index;
  int _num_transforms;
  int _max_simultaneous_transforms;

  // Even though we don't store the actual blend table data in a
  // CycleData structure, we do need to keep a local cache of the
  // relevant modified stamps there, so it can be updated per-thread.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    UpdateSeq _modified;
    UpdateSeq _global_modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  void recompute_modified(CDWriter &cdata);
  void clear_modified();

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
    register_type(_type_handle, "TransformBlendTable",
                  TypedWritableReferenceCount::get_class_type());
    // The _palette_type_handle is defined only to support older bam
    // files, generated before we renamed the type to TransformBlendTable.
    register_type(_palette_type_handle, "TransformBlendPalette",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static TypeHandle _palette_type_handle;

  friend class VertexTransform;
};

INLINE ostream &operator << (ostream &out, const TransformBlendTable &obj);

#include "transformBlendTable.I"

#endif

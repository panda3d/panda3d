// Filename: transformPalette.h
// Created by:  drose (23Mar05)
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

#ifndef TRANSFORMPALETTE_H
#define TRANSFORMPALETTE_H

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


////////////////////////////////////////////////////////////////////
//       Class : TransformPalette
// Description : Stores the total set of VertexTransforms that the
//               vertices in a particular GeomVertexData object might
//               depend on.
//
//               This structure is used for a GeomVertexData set up to
//               compute its dynamic vertices on the graphics card.
//               See TransformBlendPalette for one set up to compute
//               its dynamic vertices on the CPU.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformPalette : public TypedWritableReferenceCount {
PUBLISHED:
  TransformPalette();
  TransformPalette(const TransformPalette &copy);
  void operator = (const TransformPalette &copy);
  virtual ~TransformPalette();

  INLINE bool is_registered() const;
  INLINE static CPT(TransformPalette) register_palette(TransformPalette *palette);

  INLINE int get_num_transforms() const;
  INLINE const VertexTransform *get_transform(int n) const;
  INLINE UpdateSeq get_modified() const;

  void set_transform(int n, VertexTransform *transform);
  void remove_transform(int n);
  int add_transform(VertexTransform *transform);

  void write(ostream &out) const;

private:
  void do_register();
  void do_unregister();
  INLINE void update_modified(UpdateSeq modified);

private:
  bool _is_registered;

  typedef pvector< PT(VertexTransform) > Transforms;
  Transforms _transforms;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    UpdateSeq _modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "TransformPalette",
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

INLINE ostream &operator << (ostream &out, const TransformPalette &obj);

#include "transformPalette.I"

#endif

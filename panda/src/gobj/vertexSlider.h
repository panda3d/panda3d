// Filename: vertexSlider.h
// Created by:  drose (28Mar05)
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

#ifndef VERTEXSLIDER_H
#define VERTEXSLIDER_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "internalName.h"
#include "updateSeq.h"
#include "pset.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class SliderTable;

////////////////////////////////////////////////////////////////////
//       Class : VertexSlider
// Description : This is an abstract base class that retains some
//               slider value, which is a linear value that typically
//               ranges from 0.0 to 1.0, and is used to control the
//               animation of morphs (blend shapes).
//
//               It is similar to VertexTransform, which keeps a full
//               4x4 transform matrix, but the VertexSlider only keeps
//               a single float value.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexSlider : public TypedWritableReferenceCount {
PUBLISHED:
  VertexSlider(const InternalName *name);
  virtual ~VertexSlider();

  INLINE const InternalName *get_name() const;

  virtual float get_slider() const=0;
  INLINE UpdateSeq get_modified() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

protected:
  void mark_modified();

private:
  CPT(InternalName) _name;

  typedef pset<SliderTable *> Tables;
  Tables _tables;

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

  static PipelineCycler<CData> _global_cycler;
  static UpdateSeq _next_modified;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "VertexSlider",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class SliderTable;
};

INLINE ostream &operator << (ostream &out, const VertexSlider &obj);

#include "vertexSlider.I"

#endif

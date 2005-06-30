// Filename: vertexTransform.h
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

#ifndef VERTEXTRANSFORM_H
#define VERTEXTRANSFORM_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "updateSeq.h"
#include "luse.h"
#include "pset.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class TransformTable;

////////////////////////////////////////////////////////////////////
//       Class : VertexTransform
// Description : This is an abstract base class that holds a pointer
//               to some transform, computed in some arbitrary way,
//               that is to be applied to vertices during rendering.
//               This is used to implement soft-skinned and animated
//               vertices.  Derived classes will define how the
//               transform is actually computed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexTransform : public TypedWritableReferenceCount {
PUBLISHED:
  VertexTransform();
  virtual ~VertexTransform();

  virtual void get_matrix(LMatrix4f &matrix) const=0;
  virtual void mult_matrix(LMatrix4f &result, const LMatrix4f &previous) const;
  virtual void accumulate_matrix(LMatrix4f &accum, float weight) const;

  INLINE UpdateSeq get_modified() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  static UpdateSeq get_next_modified();
  INLINE static UpdateSeq get_global_modified();

protected:
  void mark_modified();

private:
  typedef pset<TransformTable *> Palettes;
  Palettes _tables;

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
    register_type(_type_handle, "VertexTransform",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class TransformTable;
};

INLINE ostream &operator << (ostream &out, const VertexTransform &obj);

#include "vertexTransform.I"

#endif

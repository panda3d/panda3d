/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexTransform.h
 * @author drose
 * @date 2005-03-23
 */

#ifndef VERTEXTRANSFORM_H
#define VERTEXTRANSFORM_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "updateSeq.h"
#include "luse.h"
#include "ordered_vector.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class TransformTable;

/**
 * This is an abstract base class that holds a pointer to some transform,
 * computed in some arbitrary way, that is to be applied to vertices during
 * rendering.  This is used to implement soft-skinned and animated vertices.
 * Derived classes will define how the transform is actually computed.
 */
class EXPCL_PANDA_GOBJ VertexTransform : public TypedWritableReferenceCount {
PUBLISHED:
  VertexTransform();
  virtual ~VertexTransform();

  virtual void get_matrix(LMatrix4 &matrix) const=0;
  virtual void mult_matrix(LMatrix4 &result, const LMatrix4 &previous) const;
  virtual void accumulate_matrix(LMatrix4 &accum, PN_stdfloat weight) const;

  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(modified, get_modified);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  static UpdateSeq get_next_modified(Thread *current_thread);
  INLINE static UpdateSeq get_global_modified(Thread *current_thread);

protected:
  void mark_modified(Thread *current_thread);

private:
  typedef ov_set<TransformTable *> Palettes;
  Palettes _tables;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      VertexTransform::init_type();
      return VertexTransform::get_class_type();
    }

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

INLINE std::ostream &operator << (std::ostream &out, const VertexTransform &obj);

#include "vertexTransform.I"

#endif

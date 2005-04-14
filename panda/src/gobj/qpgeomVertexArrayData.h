// Filename: qpgeomVertexArrayData.h
// Created by:  drose (17Mar05)
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

#ifndef qpGEOMVERTEXARRAYDATA_H
#define qpGEOMVERTEXARRAYDATA_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "qpgeomVertexArrayFormat.h"
#include "qpgeomEnums.h"
#include "pta_uchar.h"
#include "updateSeq.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "pmap.h"

class PreparedGraphicsObjects;
class VertexBufferContext;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexArrayData
// Description : This is the data for one array of a GeomVertexData
//               structure.  Many GeomVertexData structures will only
//               define one array, with all data elements interleaved
//               (DirectX 8.0 and before insisted on this format);
//               some will define multiple arrays.  
//
//               DirectX calls this concept of one array a "stream".
//               It also closely correlates with the concept of a
//               vertex buffer.
//
//               This object is just a block of data.  In general, you
//               should not be directly messing with this object from
//               application code.  See GeomVertexData for the
//               organizing structure, and see
//               GeomVertexReader/Writer/Rewriter for high-level tools
//               to manipulate the actual vertex data.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexArrayData : public TypedWritableReferenceCount, public qpGeomEnums {
private:
  qpGeomVertexArrayData();

PUBLISHED:
  qpGeomVertexArrayData(const qpGeomVertexArrayFormat *array_format,
                        UsageHint usage_hint);
  qpGeomVertexArrayData(const qpGeomVertexArrayData &copy);
  void operator = (const qpGeomVertexArrayData &copy);
  virtual ~qpGeomVertexArrayData();

  INLINE const qpGeomVertexArrayFormat *get_array_format() const;

  INLINE UsageHint get_usage_hint() const;
  void set_usage_hint(UsageHint usage_hint);

  INLINE bool has_column(const InternalName *name) const;

  INLINE int get_num_vertices() const;
  bool set_num_vertices(int n);
  INLINE void clear_vertices();

  INLINE int get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;

public:
  INLINE CPTA_uchar get_data() const;
  PTA_uchar modify_data();
  void set_data(CPTA_uchar data);

  void prepare(PreparedGraphicsObjects *prepared_objects);

public:
  VertexBufferContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                                   GraphicsStateGuardianBase *gsg);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

  CPT(qpGeomVertexArrayFormat) _array_format;

  // A GeomVertexArrayData keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.
  // Each PGO conversely keeps a list (a set) of all the Geoms that
  // have been prepared there.  When either destructs, it removes
  // itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, VertexBufferContext *> Contexts;
  Contexts _contexts;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    UsageHint _usage_hint;
    PTA_uchar _data;
    UpdateSeq _modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  static void write_raw_data(Datagram &dg, const PTA_uchar &data);
  static PTA_uchar read_raw_data(DatagramIterator &source);
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
    register_type(_type_handle, "qpGeomVertexArrayData",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeomCacheManager;
  friend class qpGeomVertexData;
  friend class PreparedGraphicsObjects;
};

#include "qpgeomVertexArrayData.I"

#endif

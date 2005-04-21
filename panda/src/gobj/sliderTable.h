// Filename: sliderTable.h
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

#ifndef SLIDERTABLE_H
#define SLIDERTABLE_H

#include "pandabase.h"
#include "vertexSlider.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "pmap.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"


////////////////////////////////////////////////////////////////////
//       Class : SliderTable
// Description : Stores the total set of VertexSliders that the
//               vertices in a particular GeomVertexData object might
//               depend on.
//
//               This is similar to a TransformTable, but it stores
//               VertexSliders instead of VertexTransforms, and it
//               stores them by name instead of by index number.
//               Also, it is only used when animating vertices on the
//               CPU, since GPU's don't support morphs at this point
//               in time.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SliderTable : public TypedWritableReferenceCount {
PUBLISHED:
  SliderTable();
  SliderTable(const SliderTable &copy);
  void operator = (const SliderTable &copy);
  virtual ~SliderTable();

  INLINE bool is_registered() const;
  INLINE static CPT(SliderTable) register_table(const SliderTable *table);

  INLINE int get_num_sliders() const;
  INLINE const VertexSlider *get_slider(int n) const;

  INLINE const VertexSlider *find_slider(const InternalName *name) const;
  INLINE bool has_slider(const InternalName *name) const;
  INLINE bool is_empty() const;
  INLINE UpdateSeq get_modified() const;

  void set_slider(int n, const VertexSlider *slider);
  void remove_slider(int n);
  int add_slider(const VertexSlider *slider);

  void write(ostream &out) const;

private:
  void do_register();
  void do_unregister();
  INLINE void update_modified(UpdateSeq modified);

private:
  bool _is_registered;

  typedef pvector< CPT(VertexSlider) > Sliders;
  Sliders _sliders;

  typedef pmap< CPT(InternalName), const VertexSlider *> SlidersByName;
  SlidersByName _sliders_by_name;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

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
    register_type(_type_handle, "SliderTable",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class VertexSlider;
};

INLINE ostream &operator << (ostream &out, const SliderTable &obj);

#include "sliderTable.I"

#endif

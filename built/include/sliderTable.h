/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sliderTable.h
 * @author drose
 * @date 2005-03-28
 */

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
#include "sparseArray.h"

/**
 * Stores the total set of VertexSliders that the vertices in a particular
 * GeomVertexData object might depend on.
 *
 * This is similar to a TransformTable, but it stores VertexSliders instead of
 * VertexTransforms, and it stores them by name instead of by index number.
 * Also, it is only used when animating vertices on the CPU, since GPU's don't
 * support morphs at this point in time.
 */
class EXPCL_PANDA_GOBJ SliderTable : public TypedWritableReferenceCount {
PUBLISHED:
  SliderTable();
  SliderTable(const SliderTable &copy);
  void operator = (const SliderTable &copy);
  virtual ~SliderTable();

  INLINE bool is_registered() const;
  INLINE static CPT(SliderTable) register_table(const SliderTable *table);

  INLINE size_t get_num_sliders() const;
  INLINE const VertexSlider *get_slider(size_t n) const;
  MAKE_SEQ(get_sliders, get_num_sliders, get_slider);
  INLINE const SparseArray &get_slider_rows(size_t n) const;

  INLINE const SparseArray &find_sliders(const InternalName *name) const;
  INLINE bool has_slider(const InternalName *name) const;
  INLINE bool is_empty() const;
  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(modified, get_modified);

  void set_slider(size_t n, const VertexSlider *slider);
  void set_slider_rows(size_t n, const SparseArray &rows);
  void remove_slider(size_t n);
  size_t add_slider(const VertexSlider *slider, const SparseArray &rows);

  void write(std::ostream &out) const;

private:
  void do_register();
  void do_unregister();
  INLINE void update_modified(UpdateSeq modified, Thread *current_thread);

private:
  bool _is_registered;

  class SliderDef {
  public:
    CPT(VertexSlider) _slider;
    SparseArray _rows;
  };

  typedef pvector<SliderDef> Sliders;
  Sliders _sliders;

  typedef pmap< CPT(InternalName), SparseArray > SlidersByName;
  SlidersByName _sliders_by_name;

  static SparseArray _empty_array;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return SliderTable::get_class_type();
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

INLINE std::ostream &operator << (std::ostream &out, const SliderTable &obj);

#include "sliderTable.I"

#endif

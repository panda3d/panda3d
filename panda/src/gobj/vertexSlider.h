/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexSlider.h
 * @author drose
 * @date 2005-03-28
 */

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

/**
 * This is an abstract base class that retains some slider value, which is a
 * linear value that typically ranges from 0.0 to 1.0, and is used to control
 * the animation of morphs (blend shapes).
 *
 * It is similar to VertexTransform, which keeps a full 4x4 transform matrix,
 * but the VertexSlider only keeps a single float value.
 */
class EXPCL_PANDA_GOBJ VertexSlider : public TypedWritableReferenceCount {
PUBLISHED:
  explicit VertexSlider(const InternalName *name);
  virtual ~VertexSlider();

  INLINE const InternalName *get_name() const;
  MAKE_PROPERTY(name, get_name);

  virtual PN_stdfloat get_slider() const=0;
  INLINE UpdateSeq get_modified(Thread *current_thread = Thread::get_current_thread()) const;
  MAKE_PROPERTY(slider, get_slider);
  MAKE_PROPERTY(modified, get_modified);

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

protected:
  void mark_modified(Thread *current_thread);

protected:
  CPT(InternalName) _name;

private:
  typedef pset<SliderTable *> Tables;
  Tables _tables;

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
      return VertexSlider::get_class_type();
    }

    UpdateSeq _modified;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

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

INLINE std::ostream &operator << (std::ostream &out, const VertexSlider &obj);

#include "vertexSlider.I"

#endif

// Filename: userVertexSlider.h
// Created by:  drose (28Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef USERVERTEXSLIDER_H
#define USERVERTEXSLIDER_H

#include "pandabase.h"
#include "vertexSlider.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : UserVertexSlider
// Description : This is a specialization on VertexSlider that
//               allows the user to specify any arbitrary slider
//               valie he likes.  This is rarely used except for
//               testing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ UserVertexSlider : public VertexSlider {
PUBLISHED:
  UserVertexSlider(const string &name);
  UserVertexSlider(const InternalName *name);

  INLINE void set_slider(PN_stdfloat slider);
  virtual PN_stdfloat get_slider() const;

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return UserVertexSlider::get_class_type();
    }

    PN_stdfloat _slider;
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
    VertexSlider::init_type();
    register_type(_type_handle, "UserVertexSlider",
                  VertexSlider::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "userVertexSlider.I"

#endif

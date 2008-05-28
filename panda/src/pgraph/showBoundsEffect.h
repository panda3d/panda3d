// Filename: showBoundsEffect.h
// Created by:  drose (25Mar02)
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

#ifndef SHOWBOUNDSEFFECT_H
#define SHOWBOUNDSEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : ShowBoundsEffect
// Description : Applied to a GeomNode to cause a visible bounding
//               volume to be drawn for this node.  This is generally
//               used only during development to help identify
//               bounding volume issues.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ShowBoundsEffect : public RenderEffect {
private:
  INLINE ShowBoundsEffect();

PUBLISHED:
  static CPT(RenderEffect) make(bool tight = false);

  INLINE bool get_tight() const;

protected:
  virtual bool safe_to_combine() const;
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  bool _tight;

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
    RenderEffect::init_type();
    register_type(_type_handle, "ShowBoundsEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "showBoundsEffect.I"

#endif


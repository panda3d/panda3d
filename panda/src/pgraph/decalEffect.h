// Filename: decalEffect.h
// Created by:  drose (14Mar02)
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

#ifndef DECALEFFECT_H
#define DECALEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"

////////////////////////////////////////////////////////////////////
//       Class : DecalEffect
// Description : Applied to a GeomNode to indicate that the children
//               of this GeomNode are coplanar and should be drawn as
//               decals (eliminating Z-fighting).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DecalEffect : public RenderEffect {
private:
  INLINE DecalEffect();

PUBLISHED:
  static CPT(RenderEffect) make();

protected:
  virtual bool safe_to_combine() const;
  virtual int compare_to_impl(const RenderEffect *other) const;

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
    register_type(_type_handle, "DecalEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "decalEffect.I"

#endif


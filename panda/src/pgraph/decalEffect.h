/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decalEffect.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef DECALEFFECT_H
#define DECALEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"

class FactoryParams;

/**
 * Applied to a GeomNode to indicate that the children of this GeomNode are
 * coplanar and should be drawn as decals (eliminating Z-fighting).
 */
class EXPCL_PANDA_PGRAPH DecalEffect : public RenderEffect {
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

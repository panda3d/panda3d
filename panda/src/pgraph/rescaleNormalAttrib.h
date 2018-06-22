/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rescaleNormalAttrib.h
 * @author drose
 * @date 2004-12-30
 */

#ifndef RESCALENORMALATTRIB_H
#define RESCALENORMALATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

/**
 * Specifies how polygons are to be drawn.
 */
class EXPCL_PANDA_PGRAPH RescaleNormalAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    // No adjustments are made to normals.
    M_none,

    // Normals are counterscaled by the transform's uniform scale, if
    // supported by the graphics API.
    M_rescale,

    // Normals are scaled to unit length; potentially expensive.
    M_normalize,

    // Normals are counterscaled in the presence of a uniform scale transform,
    // or normalized in the presence of a non-uniform scale transform.
    M_auto,
  };

private:
  INLINE RescaleNormalAttrib(Mode mode);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);
  INLINE static CPT(RenderAttrib) make_default();

  INLINE Mode get_mode() const;
  MAKE_PROPERTY(mode, get_mode);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  Mode _mode;

  // There are so few possible combinations, and it's used fairly often, so we
  // keep an array of the possible attributes.
  static CPT(RenderAttrib) _attribs[M_auto + 1];

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

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
  static void init_type();
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

EXPCL_PANDA_PGRAPH std::ostream &operator << (std::ostream &out, RescaleNormalAttrib::Mode mode);
EXPCL_PANDA_PGRAPH std::istream &operator >> (std::istream &in, RescaleNormalAttrib::Mode &mode);

#include "rescaleNormalAttrib.I"

#endif

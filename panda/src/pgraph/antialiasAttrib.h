/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file antialiasAttrib.h
 * @author drose
 * @date 2005-01-26
 */

#ifndef ANTIALIASATTRIB_H
#define ANTIALIASATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

/**
 * Specifies whether or how to enable antialiasing, if supported by the
 * backend renderer.
 */
class EXPCL_PANDA_PGRAPH AntialiasAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_none        = 0x0000,
    M_point       = 0x0001,
    M_line        = 0x0002,
    M_polygon     = 0x0004,
    M_multisample = 0x0008,
    M_auto        = 0x001f,
    M_type_mask   = 0x001f,

    // Extra add-on bits for performancequality hints.
    M_faster      = 0x0020,
    M_better      = 0x0040,
    M_dont_care   = 0x0060,
  };

private:
  INLINE AntialiasAttrib(unsigned short mode);

PUBLISHED:
  static CPT(RenderAttrib) make(unsigned short mode);
  static CPT(RenderAttrib) make_default();

  INLINE unsigned short get_mode() const;
  INLINE unsigned short get_mode_type() const;
  INLINE unsigned short get_mode_quality() const;

PUBLISHED:
  MAKE_PROPERTY(mode, get_mode);
  MAKE_PROPERTY(mode_type, get_mode_type);
  MAKE_PROPERTY(mode_quality, get_mode_quality);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:
  unsigned short _mode;

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

#include "antialiasAttrib.I"

#endif

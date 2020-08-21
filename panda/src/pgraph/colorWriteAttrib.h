/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorWriteAttrib.h
 * @author drose
 * @date 2002-03-04
 */

#ifndef COLORWRITEATTRIB_H
#define COLORWRITEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

/**
 * Enables or disables writing to the color buffer.  This is primarily useful
 * for certain special effects in which it is important to write to the depth
 * buffer without affecting the color buffer.
 */
class EXPCL_PANDA_PGRAPH ColorWriteAttrib : public RenderAttrib {
PUBLISHED:
  enum Channels {
    // By coincidence, these bits are the same as those for
    // D3DCOLORWRITEENABLE_RED, _GREEN, _BLUE, and _ALPHA.
    C_off    = 0x000,
    C_red    = 0x001,
    C_green  = 0x002,
    C_blue   = 0x004,
    C_rgb    = 0x007,  // == C_red | C_green | C_blue
    C_alpha  = 0x008,
    C_all    = 0x00f,
  };

private:
  INLINE ColorWriteAttrib(unsigned int channels = C_all);

PUBLISHED:
  static CPT(RenderAttrib) make(unsigned int channels);
  static CPT(RenderAttrib) make_default();

  INLINE unsigned int get_channels() const;

PUBLISHED:
  MAKE_PROPERTY(channels, get_channels);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  int _channels;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "ColorWriteAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new ColorWriteAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "colorWriteAttrib.I"

#endif

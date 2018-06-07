/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transparencyAttrib.h
 * @author drose
 * @date 2002-02-28
 */

#ifndef TRANSPARENCYATTRIB_H
#define TRANSPARENCYATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

/**
 * This controls the enabling of transparency.  Simply setting an alpha
 * component to non-1 does not in itself make an object transparent; you must
 * also enable transparency mode with a suitable TransparencyAttrib.
 * Similarly, it is wasteful to render an object with a TransparencyAttrib in
 * effect unless you actually want it to be at least partially transparent
 * (and it has alpha components less than 1).
 */
class EXPCL_PANDA_PGRAPH TransparencyAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    // The first two should be specifically 0 and 1, for historical reasons
    // (NodePath::set_transparency() used to accept a boolean value, which
    // corresponded to M_none or M_alpha).
    M_none = 0,         // No transparency.
    M_alpha = 1,        // Normal transparency, panda will sort back-to-front.
    M_premultiplied_alpha, // Assume textures use premultiplied alpha.
    M_multisample,      // Uses ms buffer, alpha values modified to 1.0.
    M_multisample_mask, // Uses ms buffer, alpha values not modified.
    M_binary,           // Only writes pixels with alpha >= 0.5.
    M_dual,             // opaque parts first, then sorted transparent parts.
  };

private:
  INLINE TransparencyAttrib(Mode mode = M_none);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);
  static CPT(RenderAttrib) make_default();

  INLINE Mode get_mode() const;
  MAKE_PROPERTY(mode, get_mode);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  Mode _mode;

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
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "TransparencyAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new TransparencyAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "transparencyAttrib.I"

#endif

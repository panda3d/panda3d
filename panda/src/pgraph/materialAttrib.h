/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.h
 * @author drose
 * @date 2002-03-04
 */

#ifndef MATERIALATTRIB_H
#define MATERIALATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "material.h"

/**
 * Indicates which, if any, material should be applied to geometry.  The
 * material is used primarily to control lighting effects, and isn't necessary
 * (or useful) in the absence of lighting.
 */
class EXPCL_PANDA_PGRAPH MaterialAttrib : public RenderAttrib {
private:
  INLINE MaterialAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make(Material *material);
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  INLINE bool is_off() const;
  INLINE Material *get_material() const;

PUBLISHED:
  MAKE_PROPERTY(material, get_material);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  PT(Material) _material;

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
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "MaterialAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new MaterialAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "materialAttrib.I"

#endif

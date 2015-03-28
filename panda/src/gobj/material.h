// Filename: material.h
// Created by:  mike (09Jan97)
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

#ifndef MATERIAL_H
#define MATERIAL_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "luse.h"
#include "numeric_types.h"
#include "config_gobj.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : Material
// Description : Defines the way an object appears in the presence of
//               lighting.  A material is only necessary if lighting
//               is to be enabled; otherwise, the material isn't used.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ Material : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  INLINE explicit Material(const string &name = "");
  INLINE Material(const Material &copy);
  void operator = (const Material &copy);
  INLINE ~Material();

  INLINE static Material *get_default();

  INLINE bool has_ambient() const;
  INLINE const LColor &get_ambient() const;
  void set_ambient(const LColor &color);
  INLINE void clear_ambient();

  INLINE bool has_diffuse() const;
  INLINE const LColor &get_diffuse() const;
  void set_diffuse(const LColor &color);
  INLINE void clear_diffuse();

  INLINE bool has_specular() const;
  INLINE const LColor &get_specular() const;
  void set_specular(const LColor &color);
  INLINE void clear_specular();

  INLINE bool has_emission() const;
  INLINE const LColor &get_emission() const;
  void set_emission(const LColor &color);
  INLINE void clear_emission();

  INLINE PN_stdfloat get_shininess() const;
  INLINE void set_shininess(PN_stdfloat shininess);

  INLINE bool get_local() const;
  INLINE void set_local(bool local);
  INLINE bool get_twoside() const;
  INLINE void set_twoside(bool twoside);

  INLINE bool operator == (const Material &other) const;
  INLINE bool operator != (const Material &other) const;
  INLINE bool operator < (const Material &other) const;

  int compare_to(const Material &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent) const;

  INLINE bool is_attrib_locked() const;
  INLINE void set_attrib_lock();

private:
  LColor _ambient;
  LColor _diffuse;
  LColor _specular;
  LColor _emission;
  PN_stdfloat _shininess;

  static PT(Material) _default;

  enum Flags {
    F_ambient     = 0x001,
    F_diffuse     = 0x002,
    F_specular    = 0x004,
    F_emission    = 0x008,
    F_local       = 0x010,
    F_twoside     = 0x020,
    F_attrib_lock = 0x040
  };
  int _flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_Material(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Material",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const Material &m) {
  m.output(out);
  return out;
}

#include "material.I"

#endif

// Filename: material.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef MATERIAL_H
#define MATERIAL_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include <typedWritableReferenceCount.h>
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : Material
// Description : Defines the way an object appears in the presence of
//               lighting.  A material is only necessary if lighting
//               is to be enabled; otherwise, the material isn't used.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Material : public TypedWritableReferenceCount {
PUBLISHED:
  INLINE Material();
  INLINE Material(const Material &copy);
  void operator = (const Material &copy);
  INLINE ~Material();

  INLINE bool has_ambient() const;
  INLINE const Colorf &get_ambient() const;
  void set_ambient(const Colorf &color);
  INLINE void clear_ambient();

  INLINE bool has_diffuse() const;
  INLINE const Colorf &get_diffuse() const;
  void set_diffuse(const Colorf &color);
  INLINE void clear_diffuse();

  INLINE bool has_specular() const;
  INLINE const Colorf &get_specular() const;
  void set_specular(const Colorf &color);
  INLINE void clear_specular();

  INLINE bool has_emission() const;
  INLINE const Colorf &get_emission() const;
  void set_emission(const Colorf &color);
  INLINE void clear_emission();

  INLINE float get_shininess() const;
  INLINE void set_shininess(float shininess);

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

private:
  Colorf _ambient;
  Colorf _diffuse;
  Colorf _specular;
  Colorf _emission;
  float _shininess;

  enum Flags {
    F_ambient   = 0x001,
    F_diffuse   = 0x002,
    F_specular  = 0x004,
    F_emission  = 0x008,
    F_local     = 0x010,
    F_twoside   = 0x020,
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

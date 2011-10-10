// Filename: eggMaterial.h
// Created by:  drose (29Jan99)
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

#ifndef EGGMATERIAL_H
#define EGGMATERIAL_H

#include "pandabase.h"

#include "eggNode.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggMaterial
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggMaterial : public EggNode {
PUBLISHED:
  EggMaterial(const string &mref_name);
  EggMaterial(const EggMaterial &copy);

  virtual void write(ostream &out, int indent_level) const;

  enum Equivalence {
    E_attributes           = 0x001,
    E_mref_name            = 0x002,
  };

  bool is_equivalent_to(const EggMaterial &other, int eq) const;
  bool sorts_less_than(const EggMaterial &other, int eq) const;

  INLINE void set_diff(const LColor &diff);
  INLINE void clear_diff();
  INLINE bool has_diff() const;
  INLINE LColor get_diff() const;

  INLINE void set_amb(const LColor &amb);
  INLINE void clear_amb();
  INLINE bool has_amb() const;
  INLINE LColor get_amb() const;

  INLINE void set_emit(const LColor &emit);
  INLINE void clear_emit();
  INLINE bool has_emit() const;
  INLINE LColor get_emit() const;

  INLINE void set_spec(const LColor &spec);
  INLINE void clear_spec();
  INLINE bool has_spec() const;
  INLINE LColor get_spec() const;

  INLINE void set_shininess(double shininess);
  INLINE void clear_shininess();
  INLINE bool has_shininess() const;
  INLINE double get_shininess() const;

  INLINE void set_local(bool local);
  INLINE void clear_local();
  INLINE bool has_local() const;
  INLINE bool get_local() const;

private:
  enum Flags {
    F_diff      = 0x001,
    F_amb       = 0x002,
    F_emit      = 0x004,
    F_spec      = 0x008,
    F_shininess = 0x010,
    F_local     = 0x020
  };

  LColor _diff;
  LColor _amb;
  LColor _emit;
  LColor _spec;
  double _shininess;
  bool _local;
  int _flags;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNode::init_type();
    register_type(_type_handle, "EggMaterial",
                  EggNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : UniqueEggMaterials
// Description : An STL function object for sorting materials into
//               order by properties.  Returns true if the two
//               referenced EggMaterial pointers are in sorted order,
//               false otherwise.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG UniqueEggMaterials {
public:
  INLINE UniqueEggMaterials(int eq = ~0);
  INLINE bool operator ()(const EggMaterial *t1, const EggMaterial *t2) const;

  int _eq;
};

#include "eggMaterial.I"

#endif

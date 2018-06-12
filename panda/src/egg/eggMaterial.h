/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterial.h
 * @author drose
 * @date 1999-01-29
 */

#ifndef EGGMATERIAL_H
#define EGGMATERIAL_H

#include "pandabase.h"

#include "eggNode.h"

#include "luse.h"

/**
 *
 */
class EXPCL_PANDA_EGG EggMaterial : public EggNode {
PUBLISHED:
  explicit EggMaterial(const std::string &mref_name);
  EggMaterial(const EggMaterial &copy);

  virtual void write(std::ostream &out, int indent_level) const;

  enum Equivalence {
    E_attributes           = 0x001,
    E_mref_name            = 0x002,
  };

  bool is_equivalent_to(const EggMaterial &other, int eq) const;
  bool sorts_less_than(const EggMaterial &other, int eq) const;

  INLINE void set_base(const LColor &base);
  INLINE void clear_base();
  INLINE bool has_base() const;
  INLINE LColor get_base() const;

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

  INLINE void set_roughness(double roughness);
  INLINE void clear_roughness();
  INLINE bool has_roughness() const;
  INLINE double get_roughness() const;

  INLINE void set_metallic(double metallic);
  INLINE void clear_metallic();
  INLINE bool has_metallic() const;
  INLINE double get_metallic() const;

  INLINE void set_ior(double ior);
  INLINE void clear_ior();
  INLINE bool has_ior() const;
  INLINE double get_ior() const;

  INLINE void set_local(bool local);
  INLINE void clear_local();
  INLINE bool has_local() const;
  INLINE bool get_local() const;

PUBLISHED:
  MAKE_PROPERTY2(base, has_base, get_base, set_base, clear_base);
  MAKE_PROPERTY2(diff, has_diff, get_diff, set_diff, clear_diff);
  MAKE_PROPERTY2(amb, has_amb, get_amb, set_amb, clear_amb);
  MAKE_PROPERTY2(emit, has_emit, get_emit, set_emit, clear_emit);
  MAKE_PROPERTY2(spec, has_spec, get_spec, set_spec, clear_spec);
  MAKE_PROPERTY2(shininess, has_shininess, get_shininess, set_shininess, clear_shininess);
  MAKE_PROPERTY2(roughness, has_roughness, get_roughness, set_roughness, clear_roughness);
  MAKE_PROPERTY2(metallic, has_metallic, get_metallic, set_metallic, clear_metallic);
  MAKE_PROPERTY2(ior, has_ior, get_ior, set_ior, clear_ior);

  MAKE_PROPERTY2(local, has_local, get_local, set_local, clear_local);

private:
  enum Flags {
    F_base      = 0x001,
    F_diff      = 0x002,
    F_amb       = 0x004,
    F_emit      = 0x008,
    F_spec      = 0x010,
    F_shininess = 0x020,
    F_roughness = 0x040,
    F_metallic  = 0x080,
    F_ior       = 0x100,
    F_local     = 0x200
  };

  LColor _base;
  LColor _diff;
  LColor _amb;
  LColor _emit;
  LColor _spec;
  double _shininess;
  double _roughness;
  double _metallic;
  double _ior;
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

/**
 * An STL function object for sorting materials into order by properties.
 * Returns true if the two referenced EggMaterial pointers are in sorted
 * order, false otherwise.
 */
class EXPCL_PANDA_EGG UniqueEggMaterials {
public:
  INLINE UniqueEggMaterials(int eq = ~0);
  INLINE bool operator ()(const EggMaterial *t1, const EggMaterial *t2) const;

  int _eq;
};

#include "eggMaterial.I"

#endif

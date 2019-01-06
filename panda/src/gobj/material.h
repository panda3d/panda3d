/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "luse.h"
#include "numeric_types.h"
#include "config_gobj.h"
#include "graphicsStateGuardianBase.h"

class FactoryParams;

/**
 * Defines the way an object appears in the presence of lighting.  A material
 * is only necessary if lighting is to be enabled; otherwise, the material
 * isn't used.
 *
 * There are two workflows that are supported: the "classic" workflow of
 * providing separate ambient, diffuse and specular colors, and the
 * "metalness" workflow, in which a base color is specified along with a
 * "metallic" value that indicates whether the material is a metal or a
 * dielectric.
 *
 * The size of the specular highlight can be specified by either specifying
 * the specular exponent (shininess) or by specifying a roughness value that
 * in perceptually linear in the range of 0-1.
 */
class EXPCL_PANDA_GOBJ Material : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  INLINE explicit Material(const std::string &name = "");
  INLINE Material(const Material &copy);
  void operator = (const Material &copy);
  INLINE ~Material();

  INLINE static Material *get_default();

  INLINE bool has_base_color() const;
  INLINE const LColor &get_base_color() const;
  void set_base_color(const LColor &color);
  void clear_base_color();

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
  void clear_specular();

  INLINE bool has_emission() const;
  INLINE const LColor &get_emission() const;
  void set_emission(const LColor &color);
  INLINE void clear_emission();

  INLINE PN_stdfloat get_shininess() const;
  void set_shininess(PN_stdfloat shininess);

  INLINE bool has_roughness() const;
  PN_stdfloat get_roughness() const;
  void set_roughness(PN_stdfloat roughness);

  INLINE bool has_metallic() const;
  INLINE PN_stdfloat get_metallic() const;
  void set_metallic(PN_stdfloat metallic);
  void clear_metallic();

  INLINE bool has_refractive_index() const;
  INLINE PN_stdfloat get_refractive_index() const;
  void set_refractive_index(PN_stdfloat refractive_index);

  INLINE bool get_local() const;
  INLINE void set_local(bool local);
  INLINE bool get_twoside() const;
  INLINE void set_twoside(bool twoside);

  INLINE bool operator == (const Material &other) const;
  INLINE bool operator != (const Material &other) const;
  INLINE bool operator < (const Material &other) const;

  int compare_to(const Material &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent) const;

  INLINE bool is_attrib_locked() const;
  INLINE void set_attrib_lock();

PUBLISHED:
  MAKE_PROPERTY2(base_color, has_base_color, get_base_color,
                             set_base_color, clear_base_color);
  MAKE_PROPERTY2(ambient, has_ambient, get_ambient,
                          set_ambient, clear_ambient);
  MAKE_PROPERTY2(diffuse, has_diffuse, get_diffuse,
                          set_diffuse, clear_diffuse);
  MAKE_PROPERTY2(specular, has_specular, get_specular,
                           set_specular, clear_specular);
  MAKE_PROPERTY2(emission, has_emission, get_emission,
                           set_emission, clear_emission);

  MAKE_PROPERTY(shininess, get_shininess, set_shininess);
  MAKE_PROPERTY(roughness, get_roughness, set_roughness);
  MAKE_PROPERTY(metallic, get_metallic, set_metallic);
  MAKE_PROPERTY(refractive_index, get_refractive_index,
                                  set_refractive_index);

  MAKE_PROPERTY(local, get_local, set_local);
  MAKE_PROPERTY(twoside, get_twoside, set_twoside);

protected:
  INLINE bool is_used_by_auto_shader() const;

public:
  INLINE void mark_used_by_auto_shader();
  INLINE int get_flags() const;

  enum Flags {
    F_ambient     = 0x001,
    F_diffuse     = 0x002,
    F_specular    = 0x004,
    F_emission    = 0x008,
    F_local       = 0x010,
    F_twoside     = 0x020,
    F_attrib_lock = 0x040,
    F_roughness   = 0x080,
    F_metallic    = 0x100,
    F_base_color  = 0x200,
    F_refractive_index = 0x400,
    F_used_by_auto_shader = 0x800,
  };

private:
  LColor _base_color;
  LColor _ambient;
  LColor _diffuse;
  LColor _specular;
  LColor _emission;
  PN_stdfloat _shininess;
  PN_stdfloat _roughness;
  PN_stdfloat _metallic;
  PN_stdfloat _refractive_index;

  static PT(Material) _default;

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

INLINE std::ostream &operator << (std::ostream &out, const Material &m) {
  m.output(out);
  return out;
}

#include "material.I"

#endif

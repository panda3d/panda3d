/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file material.I
 * @author mike
 * @date 1999-02-05
 */

/**
 *
 */
INLINE Material::
Material(const std::string &name) : Namable(name) {
  _base_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _ambient.set(1.0f, 1.0f, 1.0f, 1.0f);
  _diffuse.set(1.0f, 1.0f, 1.0f, 1.0f);
  _specular.set(0.0f, 0.0f, 0.0f, 1.0f);
  _emission.set(0.0f, 0.0f, 0.0f, 1.0f);
  _shininess = 0;
  _roughness = 1;
  _metallic = 0;
  _refractive_index = 1;
  _flags = 0;
}

/**
 *
 */
INLINE Material::
Material(const Material &copy) :
  Namable(copy) ,
  _base_color(copy._base_color),
  _ambient(copy._ambient),
  _diffuse(copy._diffuse),
  _specular(copy._specular),
  _emission(copy._emission),
  _shininess(copy._shininess),
  _roughness(copy._roughness),
  _metallic(copy._metallic),
  _refractive_index(copy._refractive_index),
  _flags(copy._flags & ~(F_attrib_lock | F_used_by_auto_shader)) {
}

/**
 *
 */
INLINE Material::
~Material() {
}

/**
 * Returns the default material.
 */
INLINE Material *Material::
get_default() {
  if (_default == 0) {
    _default = new Material("default");
  }
  return _default;
}

/**
 * Returns true if the base color has been explicitly set for this material,
 * false otherwise.
 */
INLINE bool Material::
has_base_color() const {
  return (_flags & F_base_color) != 0;
}

/**
 * Returns the base_color color setting, if it has been set.  If neither the
 * base color nor the metallic have been set, this returns the diffuse color.
 */
INLINE const LColor &Material::
get_base_color() const {
  if (!has_base_color() && !has_metallic()) {
    return _diffuse;
  } else {
    return _base_color;
  }
}

/**
 * Returns true if the ambient color has been explicitly set for this
 * material, false otherwise.
 */
INLINE bool Material::
has_ambient() const {
  return (_flags & F_ambient) != 0;
}

/**
 * Returns the ambient color setting, if it has been set.  Returns (0,0,0,0)
 * if the ambient color has not been set.
 */
INLINE const LColor &Material::
get_ambient() const {
  return _ambient;
}

/**
 * Removes the explicit ambient color from the material.
 */
INLINE void Material::
clear_ambient() {
  if (has_ambient() && is_used_by_auto_shader()) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _flags &= ~F_ambient;
  _ambient = _base_color;
}

/**
 * Returns true if the diffuse color has been explicitly set for this
 * material, false otherwise.
 */
INLINE bool Material::
has_diffuse() const {
  return (_flags & F_diffuse) != 0;
}

/**
 * Returns the diffuse color setting, if it has been set.  Returns (1,1,1,1)
 * if the diffuse color has not been set.
 */
INLINE const LColor &Material::
get_diffuse() const {
  return _diffuse;
}

/**
 * Removes the explicit diffuse color from the material.
 */
INLINE void Material::
clear_diffuse() {
  if (has_diffuse() && is_used_by_auto_shader()) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _flags &= ~F_diffuse;
  _diffuse = _base_color * (1 - _metallic);
}

/**
 * Returns true if the specular color has been explicitly set for this
 * material, false otherwise.
 */
INLINE bool Material::
has_specular() const {
  return (_flags & F_specular) != 0;
}

/**
 * Returns the specular color setting, if it has been set.  Returns (0,0,0,0)
 * if the specular color has not been set.
 */
INLINE const LColor &Material::
get_specular() const {
  return _specular;
}

/**
 * Returns true if the emission color has been explicitly set for this
 * material, false otherwise.
 */
INLINE bool Material::
has_emission() const {
  return (_flags & F_emission) != 0;
}

/**
 * Returns the emission color setting, if it has been set.  Returns (0,0,0,0)
 * if the emission color has not been set.
 */
INLINE const LColor &Material::
get_emission() const {
  return _emission;
}

/**
 * Removes the explicit emission color from the material.
 */
INLINE void Material::
clear_emission() {
  if (has_emission() && is_used_by_auto_shader()) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  _flags &= ~F_emission;
  _emission.set(0.0f, 0.0f, 0.0f, 0.0f);
}

/**
 * Returns the shininess exponent of the material.
 */
INLINE PN_stdfloat Material::
get_shininess() const {
  return _shininess;
}

/**
 * Returns true if the roughness has been explicitly set for this material,
 * false otherwise.
 */
INLINE bool Material::
has_roughness() const {
  return (_flags & F_roughness) != 0;
}

/**
 * Returns true if the metallic has been explicitly set for this material,
 * false otherwise.
 */
INLINE bool Material::
has_metallic() const {
  return (_flags & F_metallic) != 0;
}

/**
 * Returns the metallic setting, if it has been set.  Returns 0 if it has not
 * been set.
 */
INLINE PN_stdfloat Material::
get_metallic() const {
  return _metallic;
}

/**
 * Returns true if a refractive index has explicitly been specified for this
 * material.
 */
INLINE bool Material::
has_refractive_index() const {
  return (_flags & F_refractive_index) != 0;
}

/**
 * Returns the index of refraction, or 1 if none has been set for this
 * material.
 */
INLINE PN_stdfloat Material::
get_refractive_index() const {
  return _refractive_index;
}

/**
 * Returns the local viewer flag.  Set set_local().
 */
INLINE bool Material::
get_local() const {
  return (_flags & F_local) != 0;
}

/**
 * Sets the local viewer flag.  Set this true to enable camera-relative
 * specular highlights, or false to use orthogonal specular highlights.  The
 * default value is true.  Applications that use orthogonal projection should
 * specify false.
 */
INLINE void Material::
set_local(bool local) {
  if (is_used_by_auto_shader() && get_local() != local) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  if (local) {
    _flags |= F_local;
  } else {
    _flags &= ~F_local;
  }
}

/**
 * Returns the state of the two-sided lighting flag.  See set_twoside().
 */
INLINE bool Material::
get_twoside() const {
  return (_flags & F_twoside) != 0;
}

/**
 * Set this true to enable two-sided lighting.  When two-sided lighting is on,
 * both sides of a polygon will be lit by this material.  The default is for
 * two-sided lighting to be off, in which case only the front surface is lit.
 */
INLINE void Material::
set_twoside(bool twoside) {
  if (is_used_by_auto_shader() && get_twoside() != twoside) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
  if (twoside) {
    _flags |= F_twoside;
  } else {
    _flags &= ~F_twoside;
  }
}

/**
 *
 */
INLINE bool Material::
operator == (const Material &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool Material::
operator != (const Material &other) const {
  return compare_to(other) != 0;
}

/**
 *
 */
INLINE bool Material::
operator < (const Material &other) const {
  return compare_to(other) < 0;
}

/**
 * @deprecated This no longer has any meaning in 1.10.
 */
INLINE bool Material::
is_attrib_locked() const {
  return (_flags & F_attrib_lock) != 0;
}

/**
 * @deprecated This no longer has any meaning in 1.10.
 */
INLINE void Material::
set_attrib_lock() {
  _flags |= F_attrib_lock;
}

/**
 * Internal.  Returns true if a shader has been generated that uses this.
 */
INLINE bool Material::
is_used_by_auto_shader() const {
  return (_flags & F_attrib_lock) != 0;
}

/**
 * Called by the shader generator to indicate that a shader has been generated
 * that uses this material.
 */
INLINE void Material::
mark_used_by_auto_shader() {
  _flags |= F_used_by_auto_shader;
}

/**
 *
 */
INLINE int Material::
get_flags() const {
  // F_used_by_auto_shader is an internal flag, ignore it.
  return _flags & ~F_used_by_auto_shader;
}

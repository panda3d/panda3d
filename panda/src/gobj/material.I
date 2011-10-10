// Filename: material.I
// Created by:  mike (05Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: Material::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Material::
Material(const string &name) : Namable(name) {
  _ambient.set(1.0f, 1.0f, 1.0f, 1.0f);
  _diffuse.set(1.0f, 1.0f, 1.0f, 1.0f);
  _specular.set(0.0f, 0.0f, 0.0f, 1.0f);
  _emission.set(0.0f, 0.0f, 0.0f, 1.0f);
  _shininess = 0.0;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Material::
Material(const Material &copy) : Namable(copy) {
  operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Material::
~Material() {
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_default
//       Access: Published, Static
//  Description: Returns the default material.
////////////////////////////////////////////////////////////////////
INLINE Material *Material::
get_default() {
  if (_default == 0) {
    _default = new Material("default");
  }
  return _default;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_ambient
//       Access: Published
//  Description: Returns true if the ambient color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_ambient() const {
  return (_flags & F_ambient) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_ambient
//       Access: Published
//  Description: Returns the ambient color setting, if it has been
//               set.  Returns (0,0,0,0) if the ambient color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const LColor &Material::
get_ambient() const {
  return _ambient;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_ambient
//       Access: Published
//  Description: Removes the explicit ambient color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_ambient() {
  if (enforce_attrib_lock) {
    nassertv(!is_attrib_locked());
  }
  _flags &= ~F_ambient;
  _ambient.set(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_diffuse
//       Access: Published
//  Description: Returns true if the diffuse color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_diffuse() const {
  return (_flags & F_diffuse) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_diffuse
//       Access: Published
//  Description: Returns the diffuse color setting, if it has been
//               set.  Returns (1,1,1,1) if the diffuse color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const LColor &Material::
get_diffuse() const {
  return _diffuse;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_diffuse
//       Access: Published
//  Description: Removes the explicit diffuse color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_diffuse() {
  if (enforce_attrib_lock) {
    nassertv(!is_attrib_locked());
  }
  _flags &= ~F_diffuse;
  _diffuse.set(1.0f, 1.0f, 1.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_specular
//       Access: Published
//  Description: Returns true if the specular color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_specular() const {
  return (_flags & F_specular) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_specular
//       Access: Published
//  Description: Returns the specular color setting, if it has been
//               set.  Returns (0,0,0,0) if the specular color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const LColor &Material::
get_specular() const {
  return _specular;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_specular
//       Access: Published
//  Description: Removes the explicit specular color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_specular() {
  if (enforce_attrib_lock) {
    nassertv(!is_attrib_locked());
  }
  _flags &= ~F_specular;
  _specular.set(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_emission
//       Access: Published
//  Description: Returns true if the emission color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_emission() const {
  return (_flags & F_emission) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_emission
//       Access: Published
//  Description: Returns the emission color setting, if it has been
//               set.  Returns (0,0,0,0) if the emission color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const LColor &Material::
get_emission() const {
  return _emission;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_emission
//       Access: Published
//  Description: Removes the explicit emission color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_emission() {
  if (enforce_attrib_lock) {
    nassertv(!is_attrib_locked());
  }
  _flags &= ~F_emission;
  _emission.set(0.0f, 0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_shininess
//       Access: Published
//  Description: Returns the shininess exponent of the material.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Material::
get_shininess() const {
  return _shininess;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_shininess
//       Access: Published
//  Description: Sets the shininess exponent of the material.  This
//               controls the size of the specular highlight spot.  In
//               general, larger number produce a smaller specular
//               highlight, which makes the object appear shinier.
//               Smaller numbers produce a larger highlight, which
//               makes the object appear less shiny.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_shininess(PN_stdfloat shininess) {
  _shininess = shininess;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_local
//       Access: Published
//  Description: Returns the local viewer flag.  Set set_local().
////////////////////////////////////////////////////////////////////
INLINE bool Material::
get_local() const {
  return (_flags & F_local) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_local
//       Access: Published
//  Description: Sets the local viewer flag.  Set this true to enable
//               camera-relative specular highlights, or false to use
//               orthogonal specular highlights.  The default value is
//               true.  Applications that use orthogonal projection
//               should specify false.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_local(bool local) {
  if (enforce_attrib_lock) {
    nassertv(!is_attrib_locked());
  }
  if (local) {
    _flags |= F_local;
  } else {
    _flags &= ~F_local;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_twoside
//       Access: Published
//  Description: Returns the state of the two-sided lighting flag.
//               See set_twoside().
////////////////////////////////////////////////////////////////////
INLINE bool Material::
get_twoside() const {
  return (_flags & F_twoside) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_twoside
//       Access: Published
//  Description: Set this true to enable two-sided lighting.  When
//               two-sided lighting is on, both sides of a polygon
//               will be lit by this material.  The default is for
//               two-sided lighting to be off, in which case only the
//               front surface is lit.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_twoside(bool twoside) {
  if (enforce_attrib_lock) {
    nassertv(!is_attrib_locked());
  }
  if (twoside) {
    _flags |= F_twoside;
  } else {
    _flags &= ~F_twoside;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Material::operator ==
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
operator == (const Material &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
operator != (const Material &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::operator <
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
operator < (const Material &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::is_attrib_locked
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
is_attrib_locked() const {
  return (_flags & F_attrib_lock) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_attrib_lock
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_attrib_lock() {
  _flags |= F_attrib_lock;
}

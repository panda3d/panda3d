// Filename: material.I
// Created by:  mike (05Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Material::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Material::
Material() {
  _flags = 0;
  _shininess = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Material::
Material(const Material &copy) {
  operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Material::
~Material() {
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_ambient
//       Access: Public
//  Description: Returns true if the ambient color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_ambient() const {
  return (_flags & F_ambient) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_ambient
//       Access: Public
//  Description: Returns the ambient color setting, if it has been
//               set.  Returns (0,0,0,0) if the ambient color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &Material::
get_ambient() const {
  return (_flags & F_ambient) != 0 ? _ambient : Colorf::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_ambient
//       Access: Public
//  Description: Removes the explicit ambient color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_ambient() {
  _flags &= ~F_ambient;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_diffuse
//       Access: Public
//  Description: Returns true if the diffuse color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_diffuse() const {
  return (_flags & F_diffuse) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_diffuse
//       Access: Public
//  Description: Returns the diffuse color setting, if it has been
//               set.  Returns (0,0,0,0) if the diffuse color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &Material::
get_diffuse() const {
  return (_flags & F_diffuse) != 0 ? _diffuse : Colorf::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_diffuse
//       Access: Public
//  Description: Removes the explicit diffuse color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_diffuse() {
  _flags &= ~F_diffuse;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_specular
//       Access: Public
//  Description: Returns true if the specular color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_specular() const {
  return (_flags & F_specular) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_specular
//       Access: Public
//  Description: Returns the specular color setting, if it has been
//               set.  Returns (0,0,0,0) if the specular color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &Material::
get_specular() const {
  return (_flags & F_specular) != 0 ? _specular : Colorf::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_specular
//       Access: Public
//  Description: Removes the explicit specular color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_specular() {
  _flags &= ~F_specular;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::has_emission
//       Access: Public
//  Description: Returns true if the emission color has been explicitly
//               set for this material, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Material::
has_emission() const {
  return (_flags & F_emission) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_emission
//       Access: Public
//  Description: Returns the emmission color setting, if it has been
//               set.  Returns (0,0,0,0) if the emmission color has not
//               been set.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &Material::
get_emission() const {
  return (_flags & F_emission) != 0 ? _emission : Colorf::zero();
}

////////////////////////////////////////////////////////////////////
//     Function: Material::clear_emission
//       Access: Public
//  Description: Removes the explicit emission color from the material.
////////////////////////////////////////////////////////////////////
INLINE void Material::
clear_emission() {
  _flags &= ~F_emission;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_shininess
//       Access: Public
//  Description: Returns the shininess exponent of the material.
////////////////////////////////////////////////////////////////////
INLINE float Material::
get_shininess() const {
  return _shininess;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_shininess
//       Access: Public
//  Description: Sets the shininess exponent of the material.  This
//               controls the size of the specular highlight spot.  In
//               general, larger number produce a smaller specular
//               highlight, which makes the object appear shinier.
//               Smaller numbers produce a larger highlight, which
//               makes the object appear less shiny.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_shininess(float shininess) {
  _shininess = shininess;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_local
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
get_local() const {
  return (_flags & F_local) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_local
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_local(bool local) {
  if (local) {
    _flags |= F_local;
  } else {
    _flags &= ~F_local;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Material::get_twoside
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
get_twoside() const {
  return (_flags & F_twoside) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_twoside
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_twoside(bool twoside) {
  if (twoside) {
    _flags |= F_twoside;
  } else {
    _flags &= ~F_twoside;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Material::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
operator == (const Material &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
operator != (const Material &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Material::
operator < (const Material &other) const {
  return compare_to(other) < 0;
}

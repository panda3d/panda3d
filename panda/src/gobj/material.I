// Filename: material.I
// Created by:  mike (05Feb99)
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
//     Function: Material::set_ambient
//       Access: Public
//  Description: Specifies the ambient color setting of the material.
//               This will be the multiplied by any ambient lights in
//               effect on the material to set its base color.
//
//               This is the color of the object as it appears in the
//               absence of direct light.
//
//               If this is not set, the object color will be used.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_ambient(const Colorf &color) {
  _ambient = color;
  _flags |= F_ambient;
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
//     Function: Material::set_diffuse
//       Access: Public
//  Description: Specifies the diffuse color setting of the material.
//               This will be multiplied by any lights in effect on
//               the material to get the color in the parts of the
//               object illuminated by the lights.
//
//               This is the primary color of an object; the color of
//               the object as it appears in direct light, in the
//               absence of highlights.
//
//               If this is not set, the object color will be used.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_diffuse(const Colorf &color) {
  _diffuse = color;
  _flags |= F_diffuse;
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
//     Function: Material::set_specular
//       Access: Public
//  Description: Specifies the diffuse color setting of the material.
//               This will be multiplied by any lights in effect on
//               the material to compute the color of specular
//               highlights on the object.
//
//               This is the highlight color of an object: the color
//               of small highlight reflections.
//
//               If this is not set, highlights will not appear.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_specular(const Colorf &color) {
  _specular = color;
  _flags |= F_specular;
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
//     Function: Material::set_emission
//       Access: Public
//  Description: Specifies the emission color setting of the material.
//               This is the color of the object as it appears in the
//               absence of any light whatsover, including ambient
//               light.  It is as if the object is glowing by this
//               color (although of course it will not illuminate
//               neighboring objects).
//
//               If this is not set, the object will not glow by its
//               own light and will only appear visible in the
//               presence of one or more lights.
////////////////////////////////////////////////////////////////////
INLINE void Material::
set_emission(const Colorf &color) {
  _emission = color;
  _flags |= F_emission;
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
  _flags |= F_local;
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
  _flags = F_twoside;
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

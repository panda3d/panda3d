// Filename: material.cxx
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

#include "pandabase.h"
#include "material.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle Material::_type_handle;
PT(Material) Material::_default;

////////////////////////////////////////////////////////////////////
//     Function: Material::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Material::
operator = (const Material &copy) {
  Namable::operator = (copy);
  _ambient = copy._ambient;
  _diffuse = copy._diffuse;
  _specular = copy._specular;
  _emission = copy._emission;
  _shininess = copy._shininess;
  _flags = copy._flags & (~F_attrib_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_ambient
//       Access: Published
//  Description: Specifies the ambient color setting of the material.
//               This will be the multiplied by any ambient lights in
//               effect on the material to set its base color.
//
//               This is the color of the object as it appears in the
//               absence of direct light.
//
//               If this is not set, the object color will be used.
////////////////////////////////////////////////////////////////////
void Material::
set_ambient(const LColor &color) {
  if (enforce_attrib_lock) {
    if ((_flags & F_ambient)==0) {
      nassertv(!is_attrib_locked());
    }
  }
  _ambient = color;
  _flags |= F_ambient;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_diffuse
//       Access: Published
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
void Material::
set_diffuse(const LColor &color) {
  if (enforce_attrib_lock) {
    if ((_flags & F_diffuse)==0) {
      nassertv(!is_attrib_locked());
    }
  }
  _diffuse = color;
  _flags |= F_diffuse;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_specular
//       Access: Published
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
void Material::
set_specular(const LColor &color) {
  if (enforce_attrib_lock) {
    if ((_flags & F_specular)==0) {
      nassertv(!is_attrib_locked());
    }
  }
  _specular = color;
  _flags |= F_specular;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::set_emission
//       Access: Published
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
void Material::
set_emission(const LColor &color) {
  if (enforce_attrib_lock) {
    if ((_flags & F_emission)==0) {
      nassertv(!is_attrib_locked());
    }
  }
  _emission = color;
  _flags |= F_emission;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this material
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.  The
//               sorting order is arbitrary and largely meaningless,
//               except to differentiate different materials.
////////////////////////////////////////////////////////////////////
int Material::
compare_to(const Material &other) const {
  if (_flags != other._flags) {
    return _flags - other._flags;
  }
  if (has_ambient() && get_ambient() != other.get_ambient()) {
    return get_ambient().compare_to(other.get_ambient());
  }
  if (has_diffuse() && get_diffuse() != other.get_diffuse()) {
    return get_diffuse().compare_to(other.get_diffuse());
  }
  if (has_specular() && get_specular() != other.get_specular()) {
    return get_specular().compare_to(other.get_specular());
  }
  if (has_emission() && get_emission() != other.get_emission()) {
    return get_emission().compare_to(other.get_emission());
  }
  if (get_shininess() != other.get_shininess()) {
    return get_shininess() < other.get_shininess() ? -1 : 1;
  }

  return strcmp(get_name().c_str(), other.get_name().c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: Material::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Material::
output(ostream &out) const {
  out << "Material " << get_name();
  if (has_ambient()) {
    out << " a(" << get_ambient() << ")";
  }
  if (has_diffuse()) {
    out << " d(" << get_diffuse() << ")";
  }
  if (has_specular()) {
    out << " s(" << get_specular() << ")";
  }
  if (has_emission()) {
    out << " e(" << get_emission() << ")";
  }
  out << " s" << get_shininess()
      << " l" << get_local()
      << " t" << get_twoside();
}

////////////////////////////////////////////////////////////////////
//     Function: Material::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void Material::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << "Material " << get_name() << "\n";
  if (has_ambient()) {
    indent(out, indent_level + 2) << "ambient = " << get_ambient() << "\n";
  }
  if (has_diffuse()) {
    indent(out, indent_level + 2) << "diffuse = " << get_diffuse() << "\n";
  }
  if (has_specular()) {
    indent(out, indent_level + 2) << "specular = " << get_specular() << "\n";
  }
  if (has_emission()) {
    indent(out, indent_level + 2) << "emission = " << get_emission() << "\n";
  }
  indent(out, indent_level + 2) << "shininess = " << get_shininess() << "\n";
  indent(out, indent_level + 2) << "local = " << get_local() << "\n";
  indent(out, indent_level + 2) << "twoside = " << get_twoside() << "\n";
}



////////////////////////////////////////////////////////////////////
//     Function: Material::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Material object
////////////////////////////////////////////////////////////////////
void Material::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_Material);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Material::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
  _ambient.write_datagram(me);
  _diffuse.write_datagram(me);
  _specular.write_datagram(me);
  _emission.write_datagram(me);
  me.add_stdfloat(_shininess);
  me.add_int32(_flags);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::make_Material
//       Access: Protected
//  Description: Factory method to generate a Material object
////////////////////////////////////////////////////////////////////
TypedWritable *Material::
make_Material(const FactoryParams &params) {
  Material *me = new Material;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Material::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  _ambient.read_datagram(scan);
  _diffuse.read_datagram(scan);
  _specular.read_datagram(scan);
  _emission.read_datagram(scan);
  _shininess = scan.get_stdfloat();
  _flags = scan.get_int32();
}

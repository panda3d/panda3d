// Filename: material.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include "material.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

#include <stddef.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Material::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Material::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void Material::
operator = (const Material &copy) {
  _ambient = copy._ambient;
  _diffuse = copy._diffuse;
  _specular = copy._specular;
  _emission = copy._emission;
  _shininess = copy._shininess;
  _flags = copy._flags;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::compare_to
//       Access: Public
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

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Material::output 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Material::
output(ostream &out) const {
  out << "material";
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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Material::
write(ostream &out, int indent_level) const {
  bool any = false;
  if (has_ambient()) {
    indent(out, indent_level) << "ambient = " << get_ambient() << "\n";
    any = true;
  }
  if (has_diffuse()) {
    indent(out, indent_level) << "diffuse = " << get_diffuse() << "\n";
    any = true;
  }
  if (has_specular()) {
    indent(out, indent_level) << "specular = " << get_specular() << "\n";
    any = true;
  }
  if (has_emission()) {
    indent(out, indent_level) << "emission = " << get_emission() << "\n";
    any = true;
  }
  indent(out, indent_level) << "shininess = " << get_shininess() << "\n";
  indent(out, indent_level) << "local = " << get_local() << "\n";
  indent(out, indent_level) << "twoside = " << get_twoside() << "\n";
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
  _ambient.write_datagram(me);
  _diffuse.write_datagram(me);
  _specular.write_datagram(me);
  _emission.write_datagram(me);
  me.add_float32(_shininess);
  me.add_int32(_flags);
}

////////////////////////////////////////////////////////////////////
//     Function: Material::make_Material
//       Access: Protected
//  Description: Factory method to generate a Material object
////////////////////////////////////////////////////////////////////
TypedWriteable *Material::
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
fillin(DatagramIterator& scan, BamReader* manager) {
  _ambient.read_datagram(scan);
  _diffuse.read_datagram(scan);
  _specular.read_datagram(scan);
  _emission.read_datagram(scan);
  _shininess = scan.get_float32();
  _flags = scan.get_int32();
}

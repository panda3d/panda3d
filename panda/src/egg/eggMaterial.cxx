/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMaterial.cxx
 * @author drose
 * @date 1999-01-29
 */

#include "eggMaterial.h"

#include "indent.h"

TypeHandle EggMaterial::_type_handle;


/**
 *
 */
EggMaterial::
EggMaterial(const std::string &mref_name)
  : EggNode(mref_name)
{
  _flags = 0;
}

/**
 *
 */
EggMaterial::
EggMaterial(const EggMaterial &copy)
  : EggNode(copy),
    _base(copy._base),
    _diff(copy._diff),
    _amb(copy._amb),
    _emit(copy._emit),
    _spec(copy._spec),
    _shininess(copy._shininess),
    _roughness(copy._roughness),
    _metallic(copy._metallic),
    _ior(copy._ior),
    _local(copy._local),
    _flags(copy._flags)
{
}


/**
 * Writes the material definition to the indicated output stream in Egg
 * format.
 */
void EggMaterial::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Material>");

  if (has_base()) {
    indent(out, indent_level + 2)
      << "<Scalar> baser { " << get_base()[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> baseg { " << get_base()[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> baseb { " << get_base()[2] << " }\n";
    if (get_base()[3] != 1.0) {
      indent(out, indent_level + 2)
        << "<Scalar> basea { " << get_base()[3] << " }\n";
    }
  }

  if (has_diff()) {
    indent(out, indent_level + 2)
      << "<Scalar> diffr { " << get_diff()[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> diffg { " << get_diff()[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> diffb { " << get_diff()[2] << " }\n";
    if (get_diff()[3] != 1.0) {
      indent(out, indent_level + 2)
        << "<Scalar> diffa { " << get_diff()[3] << " }\n";
    }
  }

  if (has_amb()) {
    indent(out, indent_level + 2)
      << "<Scalar> ambr { " << get_amb()[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> ambg { " << get_amb()[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> ambb { " << get_amb()[2] << " }\n";
    if (get_amb()[3] != 1.0) {
      indent(out, indent_level + 2)
        << "<Scalar> amba { " << get_amb()[3] << " }\n";
    }
  }

  if (has_emit()) {
    indent(out, indent_level + 2)
      << "<Scalar> emitr { " << get_emit()[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> emitg { " << get_emit()[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> emitb { " << get_emit()[2] << " }\n";
    if (get_emit()[3] != 1.0) {
      indent(out, indent_level + 2)
        << "<Scalar> emita { " << get_emit()[3] << " }\n";
    }
  }

  if (has_spec()) {
    indent(out, indent_level + 2)
      << "<Scalar> specr { " << get_spec()[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> specg { " << get_spec()[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> specb { " << get_spec()[2] << " }\n";
    if (get_spec()[3] != 1.0) {
      indent(out, indent_level + 2)
        << "<Scalar> speca { " << get_spec()[3] << " }\n";
    }
  }

  if (has_shininess()) {
    indent(out, indent_level + 2)
      << "<Scalar> shininess { " << get_shininess() << " }\n";
  }

  if (has_roughness()) {
    indent(out, indent_level + 2)
      << "<Scalar> roughness { " << get_roughness() << " }\n";
  }

  if (has_metallic()) {
    indent(out, indent_level + 2)
      << "<Scalar> metallic { " << get_metallic() << " }\n";
  }

  if (has_ior()) {
    indent(out, indent_level + 2)
      << "<Scalar> ior { " << get_ior() << " }\n";
  }

  if (has_local()) {
    indent(out, indent_level + 2)
      << "<Scalar> local { " << get_local() << " }\n";
  }

  indent(out, indent_level) << "}\n";
}

/**
 * Returns true if the two materials are equivalent in all relevant properties
 * (according to eq), false otherwise.
 *
 * The Equivalence parameter, eq, should be set to the bitwise OR of the
 * following properties, according to what you consider relevant:
 *
 * EggMaterial::E_attributes: All material attributes (diff, spec, etc.)
 * except MRef name.
 *
 * EggMaterial::E_mref_name: The MRef name.
 */
bool EggMaterial::
is_equivalent_to(const EggMaterial &other, int eq) const {
  if (eq & E_attributes) {
    if (_flags != other._flags ||
        (has_base() && get_base() != other.get_base()) ||
        (has_diff() && get_diff() != other.get_diff()) ||
        (has_amb() && get_amb() != other.get_amb()) ||
        (has_emit() && get_emit() != other.get_emit()) ||
        (has_spec() && get_spec() != other.get_spec()) ||
        (has_shininess() && get_shininess() != other.get_shininess()) ||
        (has_roughness() && get_roughness() != other.get_roughness()) ||
        (has_metallic() && get_metallic() != other.get_metallic()) ||
        (has_ior() && get_ior() != other.get_ior()) ||
        (has_local() && get_local() != other.get_local())) {
      return false;
    }
  }

  if (eq & E_mref_name) {
    if (get_name() != other.get_name()) {
      return false;
    }
  }

  return true;
}

/**
 * An ordering operator to compare two materials for sorting order.  This
 * imposes an arbitrary ordering useful to identify unique materials,
 * according to the indicated Equivalence factor.  See is_equivalent_to().
 */
bool EggMaterial::
sorts_less_than(const EggMaterial &other, int eq) const {
  if (eq & E_attributes) {
    if (_flags != other._flags) {
      return _flags < (int)other._flags;
    }
    if (has_diff() && get_diff() != other.get_diff()) {
      return get_diff().compare_to(other.get_diff()) < 0;
    }
    if (has_amb() && get_amb() != other.get_amb()) {
      return get_amb().compare_to(other.get_amb()) < 0;
    }
    if (has_emit() && get_emit() != other.get_emit()) {
      return get_emit().compare_to(other.get_emit()) < 0;
    }
    if (has_spec() && get_spec() != other.get_spec()) {
      return get_spec().compare_to(other.get_spec()) < 0;
    }
    if (has_shininess() && get_shininess() != other.get_shininess()) {
      return get_shininess() < other.get_shininess();
    }
    if (has_roughness() && get_roughness() != other.get_roughness()) {
      return get_roughness() < other.get_roughness();
    }
    if (has_metallic() && get_metallic() != other.get_metallic()) {
      return get_metallic() < other.get_metallic();
    }
    if (has_ior() && get_ior() != other.get_ior()) {
      return get_ior() < other.get_ior();
    }
    if (has_local() && get_local() != other.get_local()) {
      return get_local() < other.get_local();
    }
  }

  if (eq & E_mref_name) {
    if (get_name() != other.get_name()) {
      return get_name() < other.get_name();
    }
  }

  return false;
}

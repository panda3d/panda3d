/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcParameter.cxx
 * @author drose
 * @date 2004-06-15
 */

#include "dcParameter.h"
#include "dcArrayParameter.h"
#include "hashGenerator.h"
#include "dcindent.h"
#include "dcTypedef.h"

using std::ostream;
using std::string;

/**
 *
 */
DCParameter::
DCParameter() {
  _typedef = nullptr;
  _has_fixed_byte_size = false;
  _has_fixed_structure = false;
  _num_nested_fields = -1;
}

/**
 *
 */
DCParameter::
DCParameter(const DCParameter &copy) :
  DCField(copy),
  _typedef(copy._typedef)
{
}

/**
 *
 */
DCParameter::
~DCParameter() {
}

/**
 *
 */
DCParameter *DCParameter::
as_parameter() {
  return this;
}

/**
 *
 */
const DCParameter *DCParameter::
as_parameter() const {
  return this;
}

/**
 *
 */
DCSimpleParameter *DCParameter::
as_simple_parameter() {
  return nullptr;
}

/**
 *
 */
const DCSimpleParameter *DCParameter::
as_simple_parameter() const {
  return nullptr;
}

/**
 *
 */
DCClassParameter *DCParameter::
as_class_parameter() {
  return nullptr;
}

/**
 *
 */
const DCClassParameter *DCParameter::
as_class_parameter() const {
  return nullptr;
}

/**
 *
 */
DCSwitchParameter *DCParameter::
as_switch_parameter() {
  return nullptr;
}

/**
 *
 */
const DCSwitchParameter *DCParameter::
as_switch_parameter() const {
  return nullptr;
}

/**
 *
 */
DCArrayParameter *DCParameter::
as_array_parameter() {
  return nullptr;
}

/**
 *
 */
const DCArrayParameter *DCParameter::
as_array_parameter() const {
  return nullptr;
}

/**
 * If this type has been referenced from a typedef, returns the DCTypedef
 * instance, or NULL if the type was declared on-the-fly.
 */
const DCTypedef *DCParameter::
get_typedef() const {
  return _typedef;
}

/**
 * Records the DCTypedef object that generated this parameter.  This is
 * normally called only from DCTypedef::make_new_parameter().
 */
void DCParameter::
set_typedef(const DCTypedef *dtypedef) {
  _typedef = dtypedef;
}

/**
 * Returns the type represented by this_type[size].
 *
 * In the case of a generic DCParameter, this means it returns a
 * DCArrayParameter wrapped around this type.
 */
DCParameter *DCParameter::
append_array_specification(const DCUnsignedIntRange &size) {
  return new DCArrayParameter(this, size);
}

/**
 *
 */
void DCParameter::
output(ostream &out, bool brief) const {
  string name;
  if (!brief) {
    name = get_name();
  }
  output_instance(out, brief, "", name, "");
}

/**
 *
 */
void DCParameter::
write(ostream &out, bool brief, int indent_level) const {
  // we must always output the name when the parameter occurs by itself within
  // a class, so we pass get_name() even if brief is true.
  write_instance(out, brief, indent_level, "", get_name(), "");
}

/**
 * Formats the parameter in the C++-like dc syntax as a typename and
 * identifier.
 */
void DCParameter::
write_instance(ostream &out, bool brief, int indent_level,
               const string &prename, const string &name,
               const string &postname) const {
  indent(out, indent_level);
  output_instance(out, brief, prename, name, postname);
  output_keywords(out);
  out << ";";
  if (!brief && _number >= 0) {
    out << "  // field " << _number;
  }
  out << "\n";
}

/**
 * Formats the instance like output_instance, but uses the typedef name
 * instead.
 */
void DCParameter::
output_typedef_name(ostream &out, bool, const string &prename,
                    const string &name, const string &postname) const {
  out << get_typedef()->get_name();
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
}

/**
 * Formats the instance like write_instance, but uses the typedef name
 * instead.
 */
void DCParameter::
write_typedef_name(ostream &out, bool brief, int indent_level,
                   const string &prename, const string &name,
                   const string &postname) const {
  indent(out, indent_level)
    << get_typedef()->get_name();
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
  output_keywords(out);
  out << ";";
  if (!brief && _number >= 0) {
    out << "  // field " << _number;
  }
  out << "\n";
}

/**
 * Accumulates the properties of this type into the hash.
 */
void DCParameter::
generate_hash(HashGenerator &hashgen) const {
  // We specifically don't call up to DCField::generate_hash(), since the
  // parameter name is not actually significant to the hash.

  if (get_num_keywords() != 0) {
    DCKeywordList::generate_hash(hashgen);
  }
}

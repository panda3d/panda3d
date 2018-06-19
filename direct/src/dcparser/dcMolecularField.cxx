/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcMolecularField.cxx
 * @author drose
 * @date 2000-10-05
 */

#include "dcMolecularField.h"
#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"



/**
 *
 */
DCMolecularField::
DCMolecularField(const std::string &name, DCClass *dclass) : DCField(name, dclass) {
  _got_keywords = false;
}

/**
 * Returns the same field pointer converted to a molecular field pointer, if
 * this is in fact a molecular field; otherwise, returns NULL.
 */
DCMolecularField *DCMolecularField::
as_molecular_field() {
  return this;
}

/**
 * Returns the same field pointer converted to a molecular field pointer, if
 * this is in fact a molecular field; otherwise, returns NULL.
 */
const DCMolecularField *DCMolecularField::
as_molecular_field() const {
  return this;
}

/**
 * Returns the number of atomic fields that make up this molecular field.
 */
int DCMolecularField::
get_num_atomics() const {
  return _fields.size();
}

/**
 * Returns the nth atomic field that makes up this molecular field.  This may
 * or may not be a field of this particular class; it might be defined in a
 * parent class.
 */
DCAtomicField *DCMolecularField::
get_atomic(int n) const {
  nassertr(n >= 0 && n < (int)_fields.size(), nullptr);
  return _fields[n];
}

/**
 * Adds the indicated atomic field to the end of the list of atomic fields
 * that make up the molecular field.  This is normally called only during
 * parsing of the dc file.  The atomic field should be fully defined by this
 * point; you should not modify the atomic field (e.g.  by adding more
 * elements) after adding it to a molecular field.
 */
void DCMolecularField::
add_atomic(DCAtomicField *atomic) {
  if (!atomic->is_bogus_field()) {
    if (!_got_keywords) {
      // The first non-bogus atomic field determines our keywords.
      copy_keywords(*atomic);
      _got_keywords = true;
    }
  }
  _fields.push_back(atomic);

  int num_atomic_fields = atomic->get_num_nested_fields();
  for (int i = 0; i < num_atomic_fields; i++) {
    _nested_fields.push_back(atomic->get_nested_field(i));
  }

  _num_nested_fields = _nested_fields.size();

  // See if we still have a fixed byte size.
  if (_has_fixed_byte_size) {
    _has_fixed_byte_size = atomic->has_fixed_byte_size();
    _fixed_byte_size += atomic->get_fixed_byte_size();
  }
  if (_has_fixed_structure) {
    _has_fixed_structure = atomic->has_fixed_structure();
  }
  if (!_has_range_limits) {
    _has_range_limits = atomic->has_range_limits();
  }
  if (!_has_default_value) {
    _has_default_value = atomic->has_default_value();
  }
  _default_value_stale = true;
}

/**
 *
 */
void DCMolecularField::
output(std::ostream &out, bool brief) const {
  out << _name;

  if (!_fields.empty()) {
    Fields::const_iterator fi = _fields.begin();
    out << " : " << (*fi)->get_name();
    ++fi;
    while (fi != _fields.end()) {
      out << ", " << (*fi)->get_name();
      ++fi;
    }
  }

  out << ";";
}

/**
 * Generates a parseable description of the object to the indicated output
 * stream.
 */
void DCMolecularField::
write(std::ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level);
  output(out, brief);
  if (!brief) {
    out << "  // field " << _number;
  }
  out << "\n";
}


/**
 * Accumulates the properties of this field into the hash.
 */
void DCMolecularField::
generate_hash(HashGenerator &hashgen) const {
  DCField::generate_hash(hashgen);

  hashgen.add_int(_fields.size());
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->generate_hash(hashgen);
  }
}

/**
 * Returns the DCPackerInterface object that represents the nth nested field.
 * This may return NULL if there is no such field (but it shouldn't do this if
 * n is in the range 0 <= n < get_num_nested_fields()).
 */
DCPackerInterface *DCMolecularField::
get_nested_field(int n) const {
  nassertr(n >= 0 && n < (int)_nested_fields.size(), nullptr);
  return _nested_fields[n];
}

/**
 * Returns true if the other interface is bitwise the same as this one--that
 * is, a uint32 only matches a uint32, etc.  Names of components, and range
 * limits, are not compared.
 */
bool DCMolecularField::
do_check_match(const DCPackerInterface *other) const {
  return other->do_check_match_molecular_field(this);
}

/**
 * Returns true if this field matches the indicated molecular field, false
 * otherwise.
 */
bool DCMolecularField::
do_check_match_molecular_field(const DCMolecularField *other) const {
  if (_nested_fields.size() != other->_nested_fields.size()) {
    return false;
  }
  for (size_t i = 0; i < _nested_fields.size(); i++) {
    if (!_nested_fields[i]->check_match(other->_nested_fields[i])) {
      return false;
    }
  }

  return true;
}

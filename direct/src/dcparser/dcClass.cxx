/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcClass.cxx
 * @author drose
 * @date 2000-10-05
 */

#include "dcClass.h"
#include "dcFile.h"
#include "dcAtomicField.h"
#include "hashGenerator.h"
#include "dcindent.h"

#include "dcClassParameter.h"
#include <algorithm>

using std::string;

#ifdef WITHIN_PANDA
#ifndef CPPPARSER
PStatCollector DCClass::_update_pcollector("App:Show code:readerPollTask:Update");
PStatCollector DCClass::_generate_pcollector("App:Show code:readerPollTask:Generate");
#endif  // CPPPARSER

ConfigVariableBool dc_multiple_inheritance
("dc-multiple-inheritance", true,
 PRC_DESC("Set this true to support multiple inheritance in the dc file.  "
          "If this is false, the old way, multiple inheritance is not "
          "supported, but field numbers will be numbered sequentially, "
          "which may be required to support old code that assumed this."));

ConfigVariableBool dc_virtual_inheritance
("dc-virtual-inheritance", true,
 PRC_DESC("Set this true to support proper virtual inheritance in the "
          "dc file, so that diamond-of-death type constructs can be used.  "
          "This also enables shadowing (overloading) of inherited method "
          "names from a base class."));

ConfigVariableBool dc_sort_inheritance_by_file
("dc-sort-inheritance-by-file", true,
 PRC_DESC("This is a temporary hack.  This should be true if you are using "
          "version 1.42 of the otp_server.exe binary, which sorted inherited "
          "fields based on the order of the classes within the DC file, "
          "rather than based on the order in which the references are made "
          "within the class."));


#endif  // WITHIN_PANDA

class SortFieldsByIndex {
public:
  inline bool operator ()(const DCField *a, const DCField *b) const {
    return a->get_number() < b->get_number();
  }
};

/**
 *
 */
DCClass::
DCClass(DCFile *dc_file, const string &name, bool is_struct, bool bogus_class) :
#ifdef WITHIN_PANDA
  _class_update_pcollector(_update_pcollector, name),
  _class_generate_pcollector(_generate_pcollector, name),
#endif
  _dc_file(dc_file),
  _name(name),
  _is_struct(is_struct),
  _bogus_class(bogus_class)
{
  _number = -1;
  _constructor = nullptr;

  _python_class_defs = nullptr;
}

/**
 *
 */
DCClass::
~DCClass() {
  if (_constructor != nullptr) {
    delete _constructor;
  }

  Fields::iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    delete (*fi);
  }
}

/**
 *
 */
DCClass *DCClass::
as_class() {
  return this;
}

/**
 *
 */
const DCClass *DCClass::
as_class() const {
  return this;
}

/**
 * Returns the number of base classes this class inherits from.
 */
int DCClass::
get_num_parents() const {
  return _parents.size();
}

/**
 * Returns the nth parent class this class inherits from.
 */
DCClass *DCClass::
get_parent(int n) const {
  nassertr(n >= 0 && n < (int)_parents.size(), nullptr);
  return _parents[n];
}

/**
 * Returns true if this class has a constructor method, false if it just uses
 * the default constructor.
 */
bool DCClass::
has_constructor() const {
  return (_constructor != nullptr);
}

/**
 * Returns the constructor method for this class if it is defined, or NULL if
 * the class uses the default constructor.
 */
DCField *DCClass::
get_constructor() const {
  return _constructor;
}

/**
 * Returns the number of fields defined directly in this class, ignoring
 * inheritance.
 */
int DCClass::
get_num_fields() const {
  return _fields.size();
}

/**
 * Returns the nth field in the class.  This is not necessarily the field with
 * index n; this is the nth field defined in the class directly, ignoring
 * inheritance.
 */
DCField *DCClass::
get_field(int n) const {
  #ifndef NDEBUG //[
  if (n < 0 || n >= (int)_fields.size()) {
    std::cerr << *this << " "
         << "n:" << n << " _fields.size():"
         << (int)_fields.size() << std::endl;
    // __asm { int 3 }
  }
  #endif //]
  nassertr_always(n >= 0 && n < (int)_fields.size(), nullptr);
  return _fields[n];
}

/**
 * Returns a pointer to the DCField that shares the indicated name.  If the
 * named field is not found in the current class, the parent classes will be
 * searched, so the value returned may not actually be a field within this
 * class.  Returns NULL if there is no such field defined.
 */
DCField *DCClass::
get_field_by_name(const string &name) const {
  FieldsByName::const_iterator ni;
  ni = _fields_by_name.find(name);
  if (ni != _fields_by_name.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    DCField *result = (*pi)->get_field_by_name(name);
    if (result != nullptr) {
      return result;
    }
  }

  // Nobody knew what this field is.
  return nullptr;
}

/**
 * Returns a pointer to the DCField that has the indicated index number.  If
 * the numbered field is not found in the current class, the parent classes
 * will be searched, so the value returned may not actually be a field within
 * this class.  Returns NULL if there is no such field defined.
 */
DCField *DCClass::
get_field_by_index(int index_number) const {
  FieldsByIndex::const_iterator ni;
  ni = _fields_by_index.find(index_number);
  if (ni != _fields_by_index.end()) {
    return (*ni).second;
  }

  // We didn't have such a field, so check our parents.
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    DCField *result = (*pi)->get_field_by_index(index_number);
    if (result != nullptr) {
      // Cache this result for future lookups.
      ((DCClass *)this)->_fields_by_index[index_number] = result;
      return result;
    }
  }

  // Nobody knew what this field is.
  return nullptr;
}

/**
 * Returns the total number of field fields defined in this class and all
 * ancestor classes.
 */
int DCClass::
get_num_inherited_fields() const {
  if (dc_multiple_inheritance && dc_virtual_inheritance &&
      _dc_file != nullptr) {
    _dc_file->check_inherited_fields();
    if (_inherited_fields.empty()) {
      ((DCClass *)this)->rebuild_inherited_fields();
    }

    // This assertion causes trouble when we are only parsing an incomplete DC
    // file.  nassertr(is_bogus_class() || !_inherited_fields.empty(), 0);
    return (int)_inherited_fields.size();

  } else {
    int num_fields = get_num_fields();

    Parents::const_iterator pi;
    for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
      num_fields += (*pi)->get_num_inherited_fields();
    }

    return num_fields;
  }
}

/**
 * Returns the nth field field in the class and all of its ancestors.
 *
 * This *used* to be the same thing as get_field_by_index(), back when the
 * fields were numbered sequentially within a class's inheritance hierarchy.
 * Now that fields have a globally unique index number, this is no longer
 * true.
 */
DCField *DCClass::
get_inherited_field(int n) const {
  if (dc_multiple_inheritance && dc_virtual_inheritance &&
      _dc_file != nullptr) {
    _dc_file->check_inherited_fields();
    if (_inherited_fields.empty()) {
      ((DCClass *)this)->rebuild_inherited_fields();
    }
    nassertr(n >= 0 && n < (int)_inherited_fields.size(), nullptr);
    return _inherited_fields[n];

  } else {
    Parents::const_iterator pi;
    for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
      int psize = (*pi)->get_num_inherited_fields();
      if (n < psize) {
        return (*pi)->get_inherited_field(n);
      }

      n -= psize;
    }

    return get_field(n);
  }
}

/**
 * Returns true if this class, or any class in the inheritance heirarchy for
 * this class, is a "bogus" class--a forward reference to an as-yet-undefined
 * class.
 */
bool DCClass::
inherits_from_bogus_class() const {
  if (is_bogus_class()) {
    return true;
  }

  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    if ((*pi)->inherits_from_bogus_class()) {
      return true;
    }
  }

  return false;
}

/**
 * Write a string representation of this instance to <out>.
 */
void DCClass::
output(std::ostream &out) const {
  if (_is_struct) {
    out << "struct";
  } else {
    out << "dclass";
  }
  if (!_name.empty()) {
    out << " " << _name;
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void DCClass::
output(std::ostream &out, bool brief) const {
  output_instance(out, brief, "", "", "");
}

/**
 * Generates a parseable description of the object to the indicated output
 * stream.
 */
void DCClass::
write(std::ostream &out, bool brief, int indent_level) const {
  indent(out, indent_level);
  if (_is_struct) {
    out << "struct";
  } else {
    out << "dclass";
  }
  if (!_name.empty()) {
    out << " " << _name;
  }

  if (!_parents.empty()) {
    Parents::const_iterator pi = _parents.begin();
    out << " : " << (*pi)->_name;
    ++pi;
    while (pi != _parents.end()) {
      out << ", " << (*pi)->_name;
      ++pi;
    }
  }

  out << " {";
  if (!brief && _number >= 0) {
    out << "  // index " << _number;
  }
  out << "\n";

  if (_constructor != nullptr) {
    _constructor->write(out, brief, indent_level + 2);
  }

  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    if (!(*fi)->is_bogus_field()) {
      (*fi)->write(out, brief, indent_level + 2);

      /*
      if (true || (*fi)->has_default_value()) {
        indent(out, indent_level + 2) << "// = ";
        DCPacker packer;
        packer.set_unpack_data((*fi)->get_default_value());
        packer.begin_unpack(*fi);
        packer.unpack_and_format(out, false);
        if (!packer.end_unpack()) {
          out << "<error>";
        }
        out << "\n";
      }
      */
    }
  }

  indent(out, indent_level) << "};\n";
}

/**
 * Generates a parseable description of the object to the indicated output
 * stream.
 */
void DCClass::
output_instance(std::ostream &out, bool brief, const string &prename,
                const string &name, const string &postname) const {
  if (_is_struct) {
    out << "struct";
  } else {
    out << "dclass";
  }
  if (!_name.empty()) {
    out << " " << _name;
  }

  if (!_parents.empty()) {
    Parents::const_iterator pi = _parents.begin();
    out << " : " << (*pi)->_name;
    ++pi;
    while (pi != _parents.end()) {
      out << ", " << (*pi)->_name;
      ++pi;
    }
  }

  out << " {";

  if (_constructor != nullptr) {
    _constructor->output(out, brief);
    out << "; ";
  }

  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    if (!(*fi)->is_bogus_field()) {
      (*fi)->output(out, brief);
      out << "; ";
    }
  }

  out << "}";
  if (!prename.empty() || !name.empty() || !postname.empty()) {
    out << " " << prename << name << postname;
  }
}

/**
 * Accumulates the properties of this class into the hash.
 */
void DCClass::
generate_hash(HashGenerator &hashgen) const {
  hashgen.add_string(_name);

  if (is_struct()) {
    hashgen.add_int(1);
  }

  hashgen.add_int(_parents.size());
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    hashgen.add_int((*pi)->get_number());
  }

  if (_constructor != nullptr) {
    _constructor->generate_hash(hashgen);
  }

  hashgen.add_int(_fields.size());
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    (*fi)->generate_hash(hashgen);
  }
}

/**
 * Empties the list of inherited fields for the class, so that it may be
 * rebuilt.  This is normally only called by
 * DCFile::rebuild_inherited_fields().
 */
void DCClass::
clear_inherited_fields() {
  _inherited_fields.clear();
}

/**
 * Recomputes the list of inherited fields for the class.
 */
void DCClass::
rebuild_inherited_fields() {
  typedef pset<string> Names;
  Names names;

  _inherited_fields.clear();

  // First, all of the inherited fields from our parent are at the top of the
  // list.
  Parents::const_iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    const DCClass *parent = (*pi);
    int num_inherited_fields = parent->get_num_inherited_fields();
    for (int i = 0; i < num_inherited_fields; ++i) {
      DCField *field = parent->get_inherited_field(i);
      if (field->get_name().empty()) {
        // Unnamed fields are always inherited.  Except in the hack case.
        if (!dc_sort_inheritance_by_file) {
          _inherited_fields.push_back(field);
        }

      } else {
        bool inserted = names.insert(field->get_name()).second;
        if (inserted) {
          // The earlier parent shadows the later parent.
          _inherited_fields.push_back(field);
        }
      }
    }
  }

  // Now add the local fields at the end of the list.  If any fields in this
  // list were already defined by a parent, we will shadow the parent
  // definition (that is, remove the parent's field from our list of inherited
  // fields).
  Fields::const_iterator fi;
  for (fi = _fields.begin(); fi != _fields.end(); ++fi) {
    DCField *field = (*fi);
    if (field->get_name().empty()) {
      // Unnamed fields are always added.
     _inherited_fields.push_back(field);

    } else {
      bool inserted = names.insert(field->get_name()).second;
      if (!inserted) {
        // This local field shadows an inherited field.  Remove the parent's
        // field from our list.
        shadow_inherited_field(field->get_name());
      }

      // Now add the local field.
      _inherited_fields.push_back(field);
    }
  }

  if (dc_sort_inheritance_by_file) {
    // Temporary hack.
    sort(_inherited_fields.begin(), _inherited_fields.end(), SortFieldsByIndex());
  }
}

/**
 * This is called only by rebuild_inherited_fields(). It removes the named
 * field from the list of _inherited_fields, presumably in preparation for
 * adding a new definition below.
 */
void DCClass::
shadow_inherited_field(const string &name) {
  Fields::iterator fi;
  for (fi = _inherited_fields.begin(); fi != _inherited_fields.end(); ++fi) {
    DCField *field = (*fi);
    if (field->get_name() == name) {
      _inherited_fields.erase(fi);
      return;
    }
  }

  // If we get here, the named field wasn't in the list.  Huh.
  nassert_raise("named field not in list");
}

/**
 * Adds the newly-allocated field to the class.  The class becomes the owner
 * of the pointer and will delete it when it destructs.  Returns true if the
 * field is successfully added, or false if there was a name conflict or some
 * other problem.
 */
bool DCClass::
add_field(DCField *field) {
  nassertr(field->get_class() == this || field->get_class() == nullptr, false);
  field->set_class(this);
  if (_dc_file != nullptr) {
    _dc_file->mark_inherited_fields_stale();
  }

  if (!field->get_name().empty()) {
    if (field->get_name() == _name) {
      // This field is a constructor.
      if (_constructor != nullptr) {
        // We already have a constructor.
        return false;
      }
      if (field->as_atomic_field() == nullptr) {
        // The constructor must be an atomic field.
        return false;
      }
      _constructor = field;
      _fields_by_name.insert
        (FieldsByName::value_type(field->get_name(), field));
      return true;
    }

    bool inserted = _fields_by_name.insert
      (FieldsByName::value_type(field->get_name(), field)).second;

    if (!inserted) {
      return false;
    }
  }

  if (_dc_file != nullptr &&
      ((dc_virtual_inheritance && dc_sort_inheritance_by_file) || !is_struct())) {
    if (dc_multiple_inheritance) {
      _dc_file->set_new_index_number(field);
    } else {
      field->set_number(get_num_inherited_fields());
    }

    bool inserted = _fields_by_index.insert
      (FieldsByIndex::value_type(field->get_number(), field)).second;

    // It shouldn't be possible for that to fail.
    nassertr(inserted, false);
  }

  _fields.push_back(field);
  return true;
}

/**
 * Adds a new parent to the inheritance hierarchy of the class.  This is
 * normally called only during parsing.
 */
void DCClass::
add_parent(DCClass *parent) {
  _parents.push_back(parent);
  _dc_file->mark_inherited_fields_stale();
}

/**
 * Assigns the unique number to this class.  This is normally called only by
 * the DCFile interface as the class is added.
 */
void DCClass::
set_number(int number) {
  _number = number;
}

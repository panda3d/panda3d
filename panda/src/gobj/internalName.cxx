/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalName.cxx
 * @author masad
 * @date 2004-07-15
 */

#include "pandabase.h"
#include "internalName.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "preparedGraphicsObjects.h"

using std::string;

PT(InternalName) InternalName::_root;
PT(InternalName) InternalName::_error;
PT(InternalName) InternalName::_default;
PT(InternalName) InternalName::_vertex;
PT(InternalName) InternalName::_normal;
PT(InternalName) InternalName::_tangent;
PT(InternalName) InternalName::_binormal;
PT(InternalName) InternalName::_texcoord;
PT(InternalName) InternalName::_color;
PT(InternalName) InternalName::_rotate;
PT(InternalName) InternalName::_size;
PT(InternalName) InternalName::_aspect_ratio;
PT(InternalName) InternalName::_transform_blend;
PT(InternalName) InternalName::_transform_weight;
PT(InternalName) InternalName::_transform_index;
PT(InternalName) InternalName::_index;
PT(InternalName) InternalName::_world;
PT(InternalName) InternalName::_camera;
PT(InternalName) InternalName::_model;
PT(InternalName) InternalName::_view;

TypeHandle InternalName::_type_handle;
TypeHandle InternalName::_texcoord_type_handle;

#ifdef HAVE_PYTHON
InternalName::PyInternTable InternalName::_py_intern_table;
#endif

InternalName::LiteralTable InternalName::_literal_table;
LightMutex InternalName::_literal_table_lock;

/**
 * Use make() to make a new InternalName instance.
 */
InternalName::
InternalName(InternalName *parent, const string &basename) :
  _parent(parent),
  _basename(basename)
{
}

/**
 *
 */
InternalName::
~InternalName() {
#ifndef NDEBUG
  if (_parent != nullptr) {
    // unref() should have removed us from our parent's table already.
    LightMutexHolder holder(_parent->_name_table_lock);
    NameTable::iterator ni = _parent->_name_table.find(_basename);
    nassertv(ni == _parent->_name_table.end());
  }
#endif
}

/**
 * This method overrides ReferenceCount::unref() to clear the pointer from its
 * parent's table when its reference count goes to zero.
 */
bool InternalName::
unref() const {
  if (_parent == nullptr) {
    // No parent; no problem.  This is the root InternalName.  Actually, this
    // probably shouldn't be destructing, but I guess it might at application
    // shutdown.
    return TypedWritableReferenceCount::unref();
  }

  LightMutexHolder holder(_parent->_name_table_lock);

  if (ReferenceCount::unref()) {
    return true;
  }

  // The reference count has just reached zero.
  NameTable::iterator ni = _parent->_name_table.find(_basename);
  nassertr(ni != _parent->_name_table.end(), false);
  _parent->_name_table.erase(ni);

  return false;
}

/**
 * Constructs a new InternalName based on this name, with the indicated string
 * following it.  This is a cheaper way to construct a hierarchical name than
 * InternalName::make(parent->get_name() + ".basename").
 */
PT(InternalName) InternalName::
append(const string &name) {
  test_ref_count_integrity();

  if (name.empty()) {
    return this;
  }

  size_t dot = name.rfind('.');
  if (dot != string::npos) {
    return append(name.substr(0, dot))->append(name.substr(dot + 1));
  }

  LightMutexHolder holder(_name_table_lock);

  NameTable::iterator ni = _name_table.find(name);
  if (ni != _name_table.end()) {
    return (*ni).second;
  }

  InternalName *internal_name = new InternalName(this, name);
  _name_table[name] = internal_name;
  return internal_name;
}

/**
 * Returns the complete name represented by the InternalName and all of its
 * parents.
 */
string InternalName::
get_name() const {
  if (_parent == get_root()) {
    return _basename;

  } else if (_parent == nullptr) {
    return string();

  } else {
    return _parent->get_name() + "." + _basename;
  }
}

/**
 * Like get_name, but uses a custom separator instead of ".".
 */
string InternalName::
join(const string &sep) const {
  if (_parent == get_root()) {
    return _basename;

  } else if (_parent == nullptr) {
    return string();

  } else {
    return _parent->join(sep) + sep + _basename;
  }
}

/**
 * Returns the index of the ancestor with the indicated basename, or -1 if no
 * ancestor has that basename.  Returns 0 if this name has the basename.
 *
 * This index value may be passed to get_ancestor() or get_net_basename() to
 * retrieve more information about the indicated name.
 */
int InternalName::
find_ancestor(const string &basename) const {
  test_ref_count_integrity();

  if (_basename == basename) {
    return 0;

  } else if (_parent != nullptr) {
    int index = _parent->find_ancestor(basename);
    if (index >= 0) {
      return index + 1;
    }
  }

  return -1;
}

/**
 * Returns the ancestor with the indicated index number.  0 is this name
 * itself, 1 is the name's parent, 2 is the parent's parent, and so on.  If
 * there are not enough ancestors, returns the root InternalName.
 */
const InternalName *InternalName::
get_ancestor(int n) const {
  test_ref_count_integrity();

  if (n == 0) {
    return this;

  } else if (_parent != nullptr) {
    return _parent->get_ancestor(n - 1);

  } else {
    return get_root();
  }
}

/**
 * Returns the oldest ancestor in the InternalName's chain, not counting the
 * root.  This will be the first name in the string, e.g.  "texcoord.foo.bar"
 * will return the InternalName "texcoord".
 */
const InternalName *InternalName::
get_top() const {
  test_ref_count_integrity();

  if (_parent != nullptr && _parent != get_root()) {
    return _parent->get_top();
  }
  return this;
}

/**
 * Returns the basename of this name prefixed by the indicated number of
 * ancestors.  0 is this name's basename, 1 is parent.basename, 2 is
 * grandparent.parent.basename, and so on.
 */
string InternalName::
get_net_basename(int n) const {
  if (n < 0) {
    return "";

  } else if (n == 0) {
    return _basename;

  } else if (_parent != nullptr && _parent != get_root()) {
    return _parent->get_net_basename(n - 1) + "." + _basename;

  } else {
    return _basename;
  }
}

/**
 *
 */
void InternalName::
output(std::ostream &out) const {
  if (_parent == get_root()) {
    out << _basename;

  } else if (_parent == nullptr) {
    out << "(root)";

  } else {
    _parent->output(out);
    out << '.' << _basename;
  }
}

/**
 * Factory method to generate a InternalName object
 */
void InternalName::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
  BamReader::get_factory()->register_factory(_texcoord_type_handle, make_texcoord_from_bam);
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void InternalName::
finalize(BamReader *) {
  // Unref the pointer that we explicitly reffed in make_from_bam().
  unref();

  // We should never get back to zero after unreffing our own count, because
  // we expect to have been stored in a pointer somewhere.  If we do get to
  // zero, it's a memory leak; the way to avoid this is to call unref_delete()
  // above instead of unref(), but this is dangerous to do from within a
  // virtual function.
  nassertv(get_ref_count() != 0);
}

/**
 * Make using a string and an integer.  Concatenates the two.
 */
PT(InternalName) InternalName::
make(const string &name, int index) {
  std::ostringstream full;
  full << name << index;
  return make(full.str());
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type InternalName is encountered in the Bam file.  It should create the
 * InternalName and extract its information from the file.
 */
TypedWritable *InternalName::
make_from_bam(const FactoryParams &params) {
  // The process of making a InternalName is slightly different than making
  // other Writable objects.  That is because all creation of InternalNames
  // should be done through the make() constructor.
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  // The name is the only thing written to the data stream.
  string name = scan.get_string();

  // Make a new InternalName with that name (or get the previous one if there
  // is one already).
  PT(InternalName) me = make(name);

  // But now we have a problem, since we have to hold the reference count and
  // there's no way to return a TypedWritable while still holding the
  // reference count!  We work around this by explicitly upping the count, and
  // also setting a finalize() callback to down it later.
  me->ref();
  manager->register_finalize(me);

  return me.p();
}

/**
 * This is a temporary method; it exists only to support old bam files (4.11
 * through 4.17) generated before we renamed this class from TexCoordName to
 * InternalName.
 */
TypedWritable *InternalName::
make_texcoord_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;
  parse_params(params, scan, manager);

  string name = scan.get_string();
  PT(InternalName) me;
  if (name == "default") {
    me = get_texcoord();
  } else {
    me = get_texcoord_name(name);
  }

  me->ref();
  manager->register_finalize(me);

  return me.p();
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void InternalName::
write_datagram(BamWriter *manager, Datagram &me) {
  me.add_string(get_name());
}

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPackerCatalog.cxx
 * @author drose
 * @date 2004-06-21
 */

#include "dcPackerCatalog.h"
#include "dcPackerInterface.h"
#include "dcPacker.h"
#include "dcSwitchParameter.h"

using std::string;

/**
 * The catalog is created only by DCPackerInterface::get_catalog().
 */
DCPackerCatalog::
DCPackerCatalog(const DCPackerInterface *root) : _root(root) {
  _live_catalog = nullptr;
}

/**
 * The copy constructor is used only internally, in update_switch_fields().
 */
DCPackerCatalog::
DCPackerCatalog(const DCPackerCatalog &copy) :
  _root(copy._root),
  _entries(copy._entries),
  _entries_by_name(copy._entries_by_name),
  _entries_by_field(copy._entries_by_field)
{
  _live_catalog = nullptr;
}

/**
 * The catalog is destroyed only by ~DCPackerInterface().
 */
DCPackerCatalog::
~DCPackerCatalog() {
  if (_live_catalog != nullptr) {
    delete _live_catalog;
  }

  SwitchCatalogs::iterator si;
  for (si = _switch_catalogs.begin(); si != _switch_catalogs.end(); ++si) {
    delete (*si).second;
  }
}

/**
 * Returns the index number of the entry with the indicated name, or -1 if no
 * entry has the indicated name.  The return value is suitable for passing to
 * get_entry().
 */
int DCPackerCatalog::
find_entry_by_name(const string &name) const {
  EntriesByName::const_iterator ni;
  ni = _entries_by_name.find(name);
  if (ni != _entries_by_name.end()) {
    return (*ni).second;
  }
  return -1;
}

/**
 * Returns the index number of the entry with the indicated field, or -1 if no
 * entry has the indicated field.  The return value is suitable for passing to
 * get_entry().
 */
int DCPackerCatalog::
find_entry_by_field(const DCPackerInterface *field) const {
  EntriesByField::const_iterator ni;
  ni = _entries_by_field.find(field);
  if (ni != _entries_by_field.end()) {
    return (*ni).second;
  }
  return -1;
}

/**
 * Returns a LiveCatalog object indicating the positions within the indicated
 * data record of each field within the catalog.  If the catalog's fields are
 * all fixed-width, this may return a statically-allocated LiveCatalog object
 * that is the same for all data records; otherwise, it will allocate a new
 * LiveCatalog object that must be freed with a later call to
 * release_live_catalog().
 */
const DCPackerCatalog::LiveCatalog *DCPackerCatalog::
get_live_catalog(const char *data, size_t length) const {
  if (_live_catalog != nullptr) {
    // Return the previously-allocated live catalog; it will be the same as
    // this one since it's based on a fixed-length field.
    return _live_catalog;
  }

  LiveCatalog *live_catalog = new LiveCatalog;
  live_catalog->_catalog = this;
  live_catalog->_live_entries.reserve(_entries.size());
  LiveCatalogEntry zero_entry;
  zero_entry._begin = 0;
  zero_entry._end = 0;
  for (size_t i = 0; i < _entries.size(); i++) {
    live_catalog->_live_entries.push_back(zero_entry);
  }

  DCPacker packer;
  packer.set_unpack_data(data, length, false);
  packer.begin_unpack(_root);
  const DCSwitchParameter *last_switch = nullptr;
  r_fill_live_catalog(live_catalog, packer, last_switch);
  bool okflag = packer.end_unpack();

  if (!okflag) {
    delete live_catalog;
    return nullptr;
  }

  if (_root->has_fixed_structure()) {
    // If our root field has a fixed structure, then the live catalog will
    // always be the same every time, so we might as well keep this one around
    // as an optimization.
    ((DCPackerCatalog *)this)->_live_catalog = live_catalog;
  }

  return live_catalog;
}

/**
 * Releases the LiveCatalog object that was returned by an earlier call to
 * get_live_catalog().  If this represents a newly-allocated live catalog, it
 * will free it; otherwise, it will do nothing.
 *
 * It is therefore always correct (and necessary) to match a call to
 * get_live_catalog() with a later call to release_live_catalog().
 */
void DCPackerCatalog::
release_live_catalog(const DCPackerCatalog::LiveCatalog *live_catalog) const {
  if (live_catalog != _live_catalog) {
    delete (LiveCatalog *)live_catalog;
  }
}

/**
 * Called only by DCPackerInterface::r_fill_catalog(), this adds a new entry
 * to the catalog.
 */
void DCPackerCatalog::
add_entry(const string &name, const DCPackerInterface *field,
          const DCPackerInterface *parent, int field_index) {
  Entry entry;
  entry._name = name;
  entry._field = field;
  entry._parent = parent;
  entry._field_index = field_index;

  int entry_index = (int)_entries.size();
  _entries.push_back(entry);
  _entries_by_field.insert(EntriesByField::value_type(field, entry_index));

  // Add an entry for the fully-qualified field name (e.g.  dna.topTex).  If
  // there was another entry for this name previously, completely replace it--
  // the fully-qualified name is supposed to be unique and trumps the local
  // field names (which are not necessarily unique).
  _entries_by_name[name] = entry_index;

  // We'll also add an entry for the local field name, for the user's
  // convenience.  This won't override a fully-qualified name that might
  // already have been recorded, and a fully-qualified name discovered later
  // that conflicts with this name will replace it.
  string local_name = field->get_name();
  if (local_name != name) {
    _entries_by_name.insert(EntriesByName::value_type(local_name, entry_index));
  }
}

/**
 * Called by DCPackerInterface to recursively fill up a newly-allocated
 * reference catalog.  Also called by update_switch_fields to append fields to
 * a catalog after a DCSwitch node is selected.
 */
void DCPackerCatalog::
r_fill_catalog(const string &name_prefix, const DCPackerInterface *field,
               const DCPackerInterface *parent, int field_index) {
  string next_name_prefix = name_prefix;

  if (parent != nullptr && !field->get_name().empty()) {
    // Record this entry in the catalog.
    next_name_prefix += field->get_name();
    add_entry(next_name_prefix, field, parent, field_index);

    next_name_prefix += ".";
  }

  const DCSwitchParameter *switch_parameter = field->as_switch_parameter();
  if (switch_parameter != nullptr) {
    // If we come upon a DCSwitch while building the catalog, save the
    // name_prefix at this point so we'll have it again when we later
    // encounter the switch while unpacking a live record (and so we can
    // return to this point in the recursion from update_switch_fields).
    _switch_prefixes[switch_parameter] = next_name_prefix;
  }

  // Add any children.
  if (field->has_nested_fields()) {
    int num_nested = field->get_num_nested_fields();
    // It's ok if num_nested is -1.
    for (int i = 0; i < num_nested; i++) {
      DCPackerInterface *nested = field->get_nested_field(i);
      if (nested != nullptr) {
        r_fill_catalog(next_name_prefix, nested, field, i);
      }
    }
  }
}

/**
 * Recursively walks through all of the fields on the catalog and fills the
 * live catalog with the appropriate offsets.
 */
void DCPackerCatalog::
r_fill_live_catalog(LiveCatalog *live_catalog, DCPacker &packer,
                    const DCSwitchParameter *&last_switch) const {
  const DCPackerInterface *current_field = packer.get_current_field();

  int field_index = live_catalog->find_entry_by_field(current_field);
  if (field_index >= 0) {
    nassertv(field_index < (int)live_catalog->_live_entries.size());
    live_catalog->_live_entries[field_index]._begin = packer.get_num_unpacked_bytes();
  }

  if (packer.has_nested_fields() &&
      (packer.get_pack_type() != PT_string && packer.get_pack_type() != PT_blob)) {
    packer.push();
    while (packer.more_nested_fields()) {
      r_fill_live_catalog(live_catalog, packer, last_switch);
    }
    packer.pop();

  } else {
    packer.unpack_skip();
  }

  if (field_index >= 0) {
    live_catalog->_live_entries[field_index]._end = packer.get_num_unpacked_bytes();
  }

  if (last_switch != packer.get_last_switch()) {
    // We've just invoked a new DCSwitch.  That means we must add the new
    // fields revealed by the switch to the reference catalog.
    last_switch = packer.get_last_switch();

    const DCPackerInterface *switch_case = packer.get_current_parent();
    nassertv(switch_case != nullptr);
    const DCPackerCatalog *switch_catalog =
      live_catalog->_catalog->update_switch_fields(last_switch, switch_case);
    nassertv(switch_catalog != nullptr);
    live_catalog->_catalog = switch_catalog;

    // And we also have to expand the live catalog to hold the new entries.
    LiveCatalogEntry zero_entry;
    zero_entry._begin = 0;
    zero_entry._end = 0;
    for (size_t i = live_catalog->_live_entries.size();
         i < switch_catalog->_entries.size();
         i++) {
      live_catalog->_live_entries.push_back(zero_entry);
    }
  }
}

/**
 * Returns a new DCPackerCatalog that includes all of the fields in this
 * object, with the addition of the fields named by switch_case.
 *
 * This is used to implement switches, which change the set of fields they
 * make available according to the data in the record, and therefore present a
 * different catalog under different circumstances.
 *
 * This returned pointer is allocated one time for each different switch_case
 * instance; if a given same switch_case is supplied twice, the same pointer
 * is returned both times.  The ownership of the returned pointer is kept by
 * this object.
 */
const DCPackerCatalog *DCPackerCatalog::
update_switch_fields(const DCSwitchParameter *switch_parameter,
                     const DCPackerInterface *switch_case) const {
  SwitchCatalogs::const_iterator si = _switch_catalogs.find(switch_case);
  if (si != _switch_catalogs.end()) {
    return (*si).second;
  }

  // Look up the name_prefix will we use for all of the fields that descend
  // from this switch.  This should be stored in this record because we must
  // have come across the DCSwitch when building the catalog the first time.
  SwitchPrefixes::const_iterator pi = _switch_prefixes.find(switch_parameter);
  if (pi == _switch_prefixes.end()) {
    // If it's not stored in the record, the switch must be hidden within some
    // non-seekable object, like an array; in this case, never mind.
    return this;
  }

  string name_prefix = (*pi).second;

  // Start by creating a new DCPackerCatalog object that contains all of the
  // fields that this one contains.
  DCPackerCatalog *switch_catalog = new DCPackerCatalog(*this);

  // Now record all of the fields of the switch case in the new catalog.  We
  // start with the second field of the switch case, since the first field
  // will be the switch parameter itself, which we would have already recorded
  // the first time around.
  int num_nested = switch_case->get_num_nested_fields();
  for (int i = 1; i < num_nested; i++) {
    DCPackerInterface *nested = switch_case->get_nested_field(i);
    if (nested != nullptr) {
      switch_catalog->r_fill_catalog(name_prefix, nested, switch_case, i);
    }
  }

  // Store the newly-generated switch catalog in the record so the same
  // pointer can be returned in the future.
  ((DCPackerCatalog *)this)->_switch_catalogs[switch_case] = switch_catalog;

  return switch_catalog;
}

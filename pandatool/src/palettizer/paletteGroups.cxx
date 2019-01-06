/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paletteGroups.cxx
 * @author drose
 * @date 2000-11-30
 */

#include "paletteGroups.h"
#include "paletteGroup.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indirectCompareNames.h"
#include "pvector.h"

TypeHandle PaletteGroups::_type_handle;

/**
 *
 */
PaletteGroups::
PaletteGroups() {
}

/**
 *
 */
PaletteGroups::
PaletteGroups(const PaletteGroups &copy) :
  _groups(copy._groups)
{
}

/**
 *
 */
void PaletteGroups::
operator = (const PaletteGroups &copy) {
  _groups = copy._groups;
}

/**
 * Inserts a new group to the set, if it is not already there.
 */
void PaletteGroups::
insert(PaletteGroup *group) {
  _groups.insert(group);
}

/**
 * Returns the number of times the given group appears in the set.  This is
 * either 1 if it appears at all, or 0 if it does not appear.
 */
PaletteGroups::size_type PaletteGroups::
count(PaletteGroup *group) const {
  return _groups.count(group);
}

/**
 * Completes the set with the transitive closure of all dependencies: for each
 * PaletteGroup already in the set a, all of the groups that it depends on are
 * added to the set, and so on.  The indicated set a may be the same as this
 * set.
 */
void PaletteGroups::
make_complete(const PaletteGroups &a) {
  Groups result;

  Groups::const_iterator gi;
  for (gi = a._groups.begin(); gi != a._groups.end(); ++gi) {
    r_make_complete(result, *gi);
  }

  _groups.swap(result);
}

/**
 * Computes the union of PaletteGroups a and b, and stores the result in this
 * object.  The result may be the same object as either a or b.
 */
void PaletteGroups::
make_union(const PaletteGroups &a, const PaletteGroups &b) {
  Groups u;

  Groups::const_iterator ai, bi;
  ai = a._groups.begin();
  bi = b._groups.begin();

  while (ai != a._groups.end() && bi != b._groups.end()) {
    if ((*ai) < (*bi)) {
      u.insert(u.end(), *ai);
      ++ai;

    } else if ((*bi) < (*ai)) {
      u.insert(u.end(), *bi);
      ++bi;

    } else { // (*ai) == (*bi)
      u.insert(u.end(), *ai);
      ++ai;
      ++bi;
    }
  }

  while (ai != a._groups.end()) {
    u.insert(u.end(), *ai);
    ++ai;
  }

  while (bi != b._groups.end()) {
    u.insert(u.end(), *bi);
    ++bi;
  }

  _groups.swap(u);
}

/**
 * Computes the intersection of PaletteGroups a and b, and stores the result
 * in this object.  The result may be the same object as either a or b.
 */
void PaletteGroups::
make_intersection(const PaletteGroups &a, const PaletteGroups &b) {
  Groups i;

  Groups::const_iterator ai, bi;
  ai = a._groups.begin();
  bi = b._groups.begin();

  while (ai != a._groups.end() && bi != b._groups.end()) {
    if ((*ai) < (*bi)) {
      ++ai;

    } else if ((*bi) < (*ai)) {
      ++bi;

    } else { // (*ai) == (*bi)
      i.insert(i.end(), *ai);
      ++ai;
      ++bi;
    }
  }

  _groups.swap(i);
}

/**
 * Removes the special "null" group from the set.  This is a special group
 * that egg files may be assigned to, but which textures never are; it
 * indicates that the egg file should not influence the palette assignment.
 */
void PaletteGroups::
remove_null() {
  Groups::iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    if ((*gi)->get_name() == "null") {
      _groups.erase(gi);
      return;
    }
  }
}

/**
 * Empties the set.
 */
void PaletteGroups::
clear() {
  _groups.clear();
}

/**
 * Returns true if the set is empty, false otherwise.
 */
bool PaletteGroups::
empty() const {
  return _groups.empty();
}

/**
 * Returns the number of elements in the set.
 */
PaletteGroups::size_type PaletteGroups::
size() const {
  return _groups.size();
}

/**
 * Returns an iterator suitable for traversing the set.
 */
PaletteGroups::iterator PaletteGroups::
begin() const {
  return _groups.begin();
}

/**
 * Returns an iterator suitable for traversing the set.
 */
PaletteGroups::iterator PaletteGroups::
end() const {
  return _groups.end();
}

/**
 *
 */
void PaletteGroups::
output(std::ostream &out) const {
  if (!_groups.empty()) {
    // Sort the group names into order by name for output.
    pvector<PaletteGroup *> group_vector;
    group_vector.reserve(_groups.size());
    Groups::const_iterator gi;
    for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
      group_vector.push_back(*gi);
    }
    sort(group_vector.begin(), group_vector.end(),
         IndirectCompareNames<PaletteGroup>());

    pvector<PaletteGroup *>::const_iterator gvi = group_vector.begin();
    out << (*gvi)->get_name();
    ++gvi;
    while (gvi != group_vector.end()) {
      out << " " << (*gvi)->get_name();
      ++gvi;
    }
  }
}

/**
 *
 */
void PaletteGroups::
write(std::ostream &out, int indent_level) const {
  // Sort the group names into order by name for output.
  pvector<PaletteGroup *> group_vector;
  group_vector.reserve(_groups.size());
  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    group_vector.push_back(*gi);
  }
  sort(group_vector.begin(), group_vector.end(),
       IndirectCompareNames<PaletteGroup>());

  pvector<PaletteGroup *>::const_iterator gvi;
  for (gvi = group_vector.begin(); gvi != group_vector.end(); ++gvi) {
    indent(out, indent_level) << (*gvi)->get_name() << "\n";
  }
}

/**
 * The recursive implementation of make_complete(), this adds the indicated
 * group and all of its dependencies to the set.
 */
void PaletteGroups::
r_make_complete(PaletteGroups::Groups &result, PaletteGroup *group) {
  bool inserted = result.insert(group).second;

  if (inserted) {
    Groups::const_iterator gi;
    for (gi = group->_dependent._groups.begin();
         gi != group->_dependent._groups.end();
         ++gi) {
      r_make_complete(result, *gi);
    }
  }
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PaletteGroups::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PaletteGroups);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void PaletteGroups::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  datagram.add_uint32(_groups.size());

  Groups::const_iterator gi;
  for (gi = _groups.begin(); gi != _groups.end(); ++gi) {
    writer->write_pointer(datagram, *gi);
  }
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int PaletteGroups::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);
  for (int i = 0; i < _num_groups; i++) {
    PaletteGroup *group;
    DCAST_INTO_R(group, p_list[pi++], i);
    _groups.insert(group);
  }
  return pi;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable* PaletteGroups::
make_PaletteGroups(const FactoryParams &params) {
  PaletteGroups *me = new PaletteGroups;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void PaletteGroups::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  _num_groups = scan.get_int32();
  manager->read_pointers(scan, _num_groups);
}

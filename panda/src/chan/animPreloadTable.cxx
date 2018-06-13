/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animPreloadTable.cxx
 * @author drose
 * @date 2008-08-05
 */

#include "animPreloadTable.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle AnimPreloadTable::_type_handle;

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) AnimPreloadTable::
make_cow_copy() {
  return new AnimPreloadTable(*this);
}

/**
 *
 */
AnimPreloadTable::
AnimPreloadTable() {
  _needs_sort = false;
}

/**
 *
 */
AnimPreloadTable::
~AnimPreloadTable() {
}

/**
 * Returns the number of animation records in the table.
 */
int AnimPreloadTable::
get_num_anims() const {
  return (int)_anims.size();
}

/**
 * Returns the index number in the table of the animation record with the
 * indicated name, or -1 if the name is not present.  By convention, the
 * basename is the filename of the egg or bam file, without the directory part
 * and without the extension.  That is, it is
 * Filename::get_basename_wo_extension().
 */
int AnimPreloadTable::
find_anim(const std::string &basename) const {
  consider_sort();
  AnimRecord record;
  record._basename = basename;
  Anims::const_iterator ai = _anims.find(record);
  if (ai != _anims.end()) {
    return int(ai - _anims.begin());
  }
  return -1;
}

/**
 * Removes all animation records from the table.
 */
void AnimPreloadTable::
clear_anims() {
  _anims.clear();
  _needs_sort = false;
}

/**
 * Removes the nth animation records from the table.  This renumbers indexes
 * for following animations.
 */
void AnimPreloadTable::
remove_anim(int n) {
  nassertv(n >= 0 && n < (int)_anims.size());
  _anims.erase(_anims.begin() + n);
}

/**
 * Adds a new animation record to the table.  If there is already a record of
 * this name, no operation is performed (the original record is unchanged).
 * See find_anim().  This will invalidate existing index numbers.
 */
void AnimPreloadTable::
add_anim(const std::string &basename, PN_stdfloat base_frame_rate, int num_frames) {
  AnimRecord record;
  record._basename = basename;
  record._base_frame_rate = base_frame_rate;
  record._num_frames = num_frames;

  _anims.push_back(record);
  _needs_sort = true;
}

/**
 * Copies the animation records from the other table into this one.  If a
 * given record name exists in both tables, the record in this one supercedes.
 */
void AnimPreloadTable::
add_anims_from(const AnimPreloadTable *other) {
  _anims.reserve(_anims.size() + other->_anims.size());
  Anims::const_iterator ai;
  for (ai = other->_anims.begin(); ai != other->_anims.end(); ++ai) {
    _anims.push_back(*ai);
  }
  _needs_sort = true;
}

/**
 *
 */
void AnimPreloadTable::
output(std::ostream &out) const {
  consider_sort();
  out << "AnimPreloadTable, " << _anims.size() << " animation records.";
}

/**
 *
 */
void AnimPreloadTable::
write(std::ostream &out, int indent_level) const {
  consider_sort();
  indent(out, indent_level)
    << "AnimPreloadTable, " << _anims.size() << " animation records:\n";
  Anims::const_iterator ai;
  for (ai = _anims.begin(); ai != _anims.end(); ++ai) {
    const AnimRecord &record = (*ai);
    indent(out, indent_level + 2)
      << record._basename << ": " << record._num_frames << " frames at "
      << record._base_frame_rate << " fps\n";
  }
}

/**
 * Factory method to generate an AnimPreloadTable object
 */
void AnimPreloadTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void AnimPreloadTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  consider_sort();

  dg.add_uint16(_anims.size());
  Anims::const_iterator ai;
  for (ai = _anims.begin(); ai != _anims.end(); ++ai) {
    const AnimRecord &record = (*ai);
    dg.add_string(record._basename);
    dg.add_stdfloat(record._base_frame_rate);
    dg.add_int32(record._num_frames);
  }
}

/**
 * Factory method to generate an AnimPreloadTable object
 */
TypedWritable *AnimPreloadTable::
make_from_bam(const FactoryParams &params) {
  AnimPreloadTable *me = new AnimPreloadTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimPreloadTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  int num_anims = scan.get_uint16();
  _anims.reserve(num_anims);
  for (int i = 0; i < num_anims; ++i) {
    AnimRecord record;
    record._basename = scan.get_string();
    record._base_frame_rate = scan.get_stdfloat();
    record._num_frames = scan.get_int32();
    _anims.push_back(record);
  }
}

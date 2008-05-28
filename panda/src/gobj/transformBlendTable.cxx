// Filename: transformBlendTable.cxx
// Created by:  drose (24Mar05)
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

#include "transformBlendTable.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle TransformBlendTable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::make_cow_copy
//       Access: Protected, Virtual
//  Description: Required to implement CopyOnWriteObject.
////////////////////////////////////////////////////////////////////
PT(CopyOnWriteObject) TransformBlendTable::
make_cow_copy() {
  return new TransformBlendTable(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TransformBlendTable::
TransformBlendTable() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TransformBlendTable::
TransformBlendTable(const TransformBlendTable &copy) :
  _blends(copy._blends),
  _rows(copy._rows)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
operator = (const TransformBlendTable &copy) {
  _blends = copy._blends;
  _rows = copy._rows;
  clear_index();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TransformBlendTable::
~TransformBlendTable() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::set_blend
//       Access: Published
//  Description: Replaces the blend at the nth position with the
//               indicated value.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
set_blend(int n, const TransformBlend &blend) {
  nassertv(n >= 0 && n < (int)_blends.size());
  _blends[n] = blend;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::remove_blend
//       Access: Published
//  Description: Removes the blend at the nth position.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
remove_blend(int n) {
  nassertv(n >= 0 && n < (int)_blends.size());
  _blends.erase(_blends.begin() + n);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::add_blend
//       Access: Published
//  Description: Adds a new blend to the table, and returns its
//               index number.  If there is already an identical blend
//               in the table, simply returns that number instead.
////////////////////////////////////////////////////////////////////
int TransformBlendTable::
add_blend(const TransformBlend &blend) {
  consider_rebuild_index();

  BlendIndex::iterator bi;
  bi = _blend_index.find(&blend);
  if (bi != _blend_index.end()) {
    // Already had it.
    return (*bi).second;
  }

  bool needs_realloc = (_blends.size() >= _blends.capacity());
  int new_position = (int)_blends.size();
  _blends.push_back(blend);

  if (needs_realloc) {
    // We just reallocated the blends vector, so we must rebuild the
    // index.
    clear_index();

  } else {
    // Since we didn't realloc the blends vector, just update it with
    // the latest.
    const TransformBlend &added_blend = _blends[new_position];
    _blend_index[&added_blend] = new_position;
    _max_simultaneous_transforms = max(_max_simultaneous_transforms,
                                       blend.get_num_transforms());

    // We can't compute this one as we go, so set it to a special
    // value to indicate it needs to be recomputed.
    _num_transforms = -1;
  }

  return new_position;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < (int)_blends.size(); i++) {
    indent(out, indent_level)
      << i << ". " << _blends[i] << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::clear_index
//       Access: Private
//  Description: Resets the index so that it will be rebuilt next time
//               it is needed.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
clear_index() {
  _blend_index.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::rebuild_index
//       Access: Private
//  Description: Rebuilds the index so that we can easily determine
//               what blend combinations are already present in the
//               table.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
rebuild_index() {
  _blend_index.clear();

  // We'll also count up these two statistics while we rebuild the
  // index.
  _num_transforms = 0;
  _max_simultaneous_transforms = 0;

  pset<const VertexTransform *> transforms;

  for (int i = 0; i < (int)_blends.size(); ++i) {
    const TransformBlend &blend = _blends[i];
    _blend_index[&blend] = i;

    for (int ti = 0; ti < blend.get_num_transforms(); ++ti) {
      transforms.insert(blend.get_transform(ti));
    }
    _max_simultaneous_transforms = max(_max_simultaneous_transforms,
                                       blend.get_num_transforms());
  }

  _num_transforms = transforms.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::recompute_modified
//       Access: Private
//  Description: Recomputes the modified stamp from the various
//               TransformBlend objects, if necessary.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
recompute_modified(TransformBlendTable::CData *cdata, Thread *current_thread) {
  // Update the global_modified sequence number first, to prevent race
  // conditions.
  cdata->_global_modified = VertexTransform::get_global_modified(current_thread);

  // Now get the local modified number.
  UpdateSeq seq;
  Blends::const_iterator bi;
  for (bi = _blends.begin(); bi != _blends.end(); ++bi) {
    seq = max(seq, (*bi).get_modified(current_thread));
  }

  cdata->_modified = seq;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::clear_modified
//       Access: Private
//  Description: Clears the modified stamp to force it to be
//               recomputed.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
clear_modified(Thread *current_thread) {
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_global_modified = UpdateSeq();
  cdata->_modified = UpdateSeq();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TransformBlendTable.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_blends.size());
  Blends::const_iterator bi;
  for (bi = _blends.begin(); bi != _blends.end(); ++bi) {
    (*bi).write_datagram(manager, dg);
  }

  _rows.write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TransformBlendTable::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  Blends::iterator bi;
  for (bi = _blends.begin(); bi != _blends.end(); ++bi) {
    pi += (*bi).complete_pointers(p_list + pi, manager);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TransformBlendTable is encountered
//               in the Bam file.  It should create the TransformBlendTable
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TransformBlendTable::
make_from_bam(const FactoryParams &params) {
  TransformBlendTable *object = new TransformBlendTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformBlendTable.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  size_t num_blends = scan.get_uint16();
  _blends.reserve(num_blends);
  size_t i;
  for (i = 0; i < num_blends; ++i) {
    TransformBlend blend;
    blend.fillin(scan, manager);
    _blends.push_back(blend);
  }

  if (manager->get_file_minor_ver() >= 7) {
    _rows.read_datagram(scan, manager);
  } else {
    // In this case, for bam files prior to 6.7, we must define the
    // SparseArray with the full number of vertices.  This is done
    // in GeomVertexData::complete_pointers().
  }

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *TransformBlendTable::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformBlendTable.
////////////////////////////////////////////////////////////////////
void TransformBlendTable::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  Thread *current_thread = Thread::get_current_thread();
  _modified = VertexTransform::get_next_modified(current_thread);
  _global_modified = VertexTransform::get_global_modified(current_thread);
}

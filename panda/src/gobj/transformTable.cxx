/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformTable.cxx
 * @author drose
 * @date 2005-03-23
 */

#include "transformTable.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle TransformTable::_type_handle;

/**
 *
 */
TransformTable::
TransformTable() :
  _is_registered(false)
{
}

/**
 *
 */
TransformTable::
TransformTable(const TransformTable &copy) :
  _is_registered(false),
  _transforms(copy._transforms)
{
}

/**
 *
 */
void TransformTable::
operator = (const TransformTable &copy) {
  nassertv(!_is_registered);
  _transforms = copy._transforms;
}

/**
 *
 */
TransformTable::
~TransformTable() {
  if (_is_registered) {
    do_unregister();
  }
}

/**
 * Replaces the nth transform.  Only valid for unregistered tables.
 */
void TransformTable::
set_transform(size_t n, const VertexTransform *transform) {
  nassertv(!_is_registered);
  nassertv(n < _transforms.size());
  _transforms[n] = transform;
}

/**
 * Inserts a new transform to the table at the given index position.  If the
 * index is beyond the end of the table, appends it to the end.  Only valid
 * for unregistered tables.
 *
 * This does not automatically uniquify the pointer; if the transform is
 * already present in the table, it will be added twice.
 */
void TransformTable::
insert_transform(size_t n, const VertexTransform *transform) {
  nassertv(!_is_registered);
  if (n > _transforms.size()) {
    n = _transforms.size();
  }
  _transforms.insert(_transforms.begin() + n, transform);
}

/**
 * Removes the nth transform.  Only valid for unregistered tables.
 */
void TransformTable::
remove_transform(size_t n) {
  nassertv(!_is_registered);
  nassertv(n < _transforms.size());
  _transforms.erase(_transforms.begin() + n);
}

/**
 * Adds a new transform to the table and returns the index number of the new
 * transform.  Only valid for unregistered tables.
 *
 * This does not automatically uniquify the pointer; if the transform is
 * already present in the table, it will be added twice.
 */
size_t TransformTable::
add_transform(const VertexTransform *transform) {
  nassertr(!_is_registered, -1);
  size_t new_index = _transforms.size();
  _transforms.push_back(transform);
  return new_index;
}

/**
 *
 */
void TransformTable::
write(std::ostream &out) const {
  for (size_t i = 0; i < _transforms.size(); ++i) {
    out << i << ". " << *_transforms[i] << "\n";
  }
}

/**
 * Called internally when the table is registered.
 */
void TransformTable::
do_register() {
  nassertv(!_is_registered);

  Transforms::iterator ti;
  for (ti = _transforms.begin(); ti != _transforms.end(); ++ti) {
    VertexTransform *transform = (VertexTransform *)(*ti).p();
    bool inserted = transform->_tables.insert(this).second;
    nassertv(inserted);
  }
  _is_registered = true;
}

/**
 * Called internally when the table is unregistered (i.e.  right before
 * destruction).
 */
void TransformTable::
do_unregister() {
  nassertv(_is_registered);

  Transforms::iterator ti;
  for (ti = _transforms.begin(); ti != _transforms.end(); ++ti) {
    VertexTransform *transform = (VertexTransform *)(*ti).p();
    transform->_tables.erase(this);
  }
  _is_registered = false;
}

/**
 * Tells the BamReader how to create objects of type TransformTable.
 */
void TransformTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TransformTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  dg.add_uint16(_transforms.size());
  for (Transforms::const_iterator ti = _transforms.begin();
       ti != _transforms.end();
       ++ti) {
    manager->write_pointer(dg, *ti);
  }

  manager->write_cdata(dg, _cycler);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int TransformTable::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (Transforms::iterator ti = _transforms.begin();
       ti != _transforms.end();
       ++ti) {
    (*ti) = DCAST(VertexTransform, p_list[pi++]);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type TransformTable is encountered in the Bam file.  It should create the
 * TransformTable and extract its information from the file.
 */
TypedWritable *TransformTable::
make_from_bam(const FactoryParams &params) {
  TransformTable *object = new TransformTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new TransformTable.
 */
void TransformTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);

  size_t num_transforms = scan.get_uint16();
  _transforms.reserve(num_transforms);
  for (size_t i = 0; i < num_transforms; ++i) {
    manager->read_pointer(scan);
    _transforms.push_back(nullptr);
  }

  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
CycleData *TransformTable::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TransformTable::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new TransformTable.
 */
void TransformTable::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  Thread *current_thread = Thread::get_current_thread();
  _modified = VertexTransform::get_next_modified(current_thread);
}

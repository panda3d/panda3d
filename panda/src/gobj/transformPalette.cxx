// Filename: transformPalette.cxx
// Created by:  drose (23Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "transformPalette.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle TransformPalette::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TransformPalette::
TransformPalette() :
  _is_registered(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TransformPalette::
TransformPalette(const TransformPalette &copy) :
  _is_registered(false),
  _transforms(copy._transforms)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformPalette::
operator = (const TransformPalette &copy) {
  nassertv(!_is_registered);
  _transforms = copy._transforms;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TransformPalette::
~TransformPalette() {
  if (_is_registered) {
    do_unregister();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::set_transform
//       Access: Published
//  Description: Replaces the nth transform.  Only valid for
//               unregistered palettes.
////////////////////////////////////////////////////////////////////
void TransformPalette::
set_transform(int n, VertexTransform *transform) {
  nassertv(!_is_registered);
  nassertv(n >= 0 && n < (int)_transforms.size());
  _transforms[n] = transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::remove_transform
//       Access: Published
//  Description: Removes the nth transform.  Only valid for
//               unregistered palettes.
////////////////////////////////////////////////////////////////////
void TransformPalette::
remove_transform(int n) {
  nassertv(!_is_registered);
  nassertv(n >= 0 && n < (int)_transforms.size());
  _transforms.erase(_transforms.begin() + n);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::add_transform
//       Access: Published
//  Description: Adds a new transform to the palette and returns the
//               index number of the new transform.  Only valid for
//               unregistered palettes.
////////////////////////////////////////////////////////////////////
int TransformPalette::
add_transform(VertexTransform *transform) {
  nassertr(!_is_registered, -1);
  int new_index = (int)_transforms.size();
  _transforms.push_back(transform);
  return new_index;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformPalette::
write(ostream &out) const {
  for (size_t i = 0; i < _transforms.size(); ++i) {
    out << i << ". " << *_transforms[i] << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::do_register
//       Access: Private
//  Description: Called internally when the palette is registered.
////////////////////////////////////////////////////////////////////
void TransformPalette::
do_register() {
  nassertv(!_is_registered);

  Transforms::iterator ti;
  for (ti = _transforms.begin(); ti != _transforms.end(); ++ti) {
    bool inserted = (*ti)->_palettes.insert(this).second;
    nassertv(inserted);
  }
  _is_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::do_unregister
//       Access: Private
//  Description: Called internally when the palette is unregistered
//               (i.e. right before destruction).
////////////////////////////////////////////////////////////////////
void TransformPalette::
do_unregister() {
  nassertv(_is_registered);

  Transforms::iterator ti;
  for (ti = _transforms.begin(); ti != _transforms.end(); ++ti) {
    (*ti)->_palettes.erase(this);
  }
  _is_registered = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TransformPalette.
////////////////////////////////////////////////////////////////////
void TransformPalette::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformPalette::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TransformPalette is encountered
//               in the Bam file.  It should create the TransformPalette
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TransformPalette::
make_from_bam(const FactoryParams &params) {
  TransformPalette *object = new TransformPalette;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformPalette.
////////////////////////////////////////////////////////////////////
void TransformPalette::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *TransformPalette::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformPalette::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int TransformPalette::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformPalette::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformPalette.
////////////////////////////////////////////////////////////////////
void TransformPalette::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}

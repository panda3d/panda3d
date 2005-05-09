// Filename: sliderTable.cxx
// Created by:  drose (28Mar05)
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

#include "sliderTable.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle SliderTable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
SliderTable::
SliderTable() :
  _is_registered(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
SliderTable::
SliderTable(const SliderTable &copy) :
  _is_registered(false),
  _sliders(copy._sliders),
  _sliders_by_name(copy._sliders_by_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SliderTable::
operator = (const SliderTable &copy) {
  nassertv(!_is_registered);
  _sliders = copy._sliders;
  _sliders_by_name = copy._sliders_by_name;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SliderTable::
~SliderTable() {
  if (_is_registered) {
    do_unregister();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::set_slider
//       Access: Published
//  Description: Replaces the nth slider.  Only valid for
//               unregistered tables.
////////////////////////////////////////////////////////////////////
void SliderTable::
set_slider(int n, const VertexSlider *slider) {
  nassertv(!_is_registered);
  nassertv(n >= 0 && n < (int)_sliders.size());

  if (_sliders[n]->get_name() != slider->get_name()) {
    // Not allowed to move a slider this way.
    nassertv(!has_slider(slider->get_name()));
  }

  _sliders[n] = slider;
  _sliders_by_name[slider->get_name()] = slider;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::remove_slider
//       Access: Published
//  Description: Removes the nth slider.  Only valid for
//               unregistered tables.
////////////////////////////////////////////////////////////////////
void SliderTable::
remove_slider(int n) {
  nassertv(!_is_registered);
  nassertv(n >= 0 && n < (int)_sliders.size());

  SlidersByName::iterator si = _sliders_by_name.find(_sliders[n]->get_name());
  nassertv(si != _sliders_by_name.end());
  _sliders_by_name.erase(si);

  _sliders.erase(_sliders.begin() + n);
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::add_slider
//       Access: Published
//  Description: Adds a new slider to the table, or replaces an
//               existing slider with the same name, and returns the
//               index number of the new slider.  Only valid for
//               unregistered tables.
////////////////////////////////////////////////////////////////////
int SliderTable::
add_slider(const VertexSlider *slider) {
  nassertr(!_is_registered, -1);

  SlidersByName::iterator sni = _sliders_by_name.find(slider->get_name());
  if (sni != _sliders_by_name.end()) {
    // We've already got a slider with this name; replace it.
    CPT(VertexSlider) orig_slider = (*sni).second;
    Sliders::iterator si = find(_sliders.begin(), _sliders.end(), orig_slider);
    nassertr(si != _sliders.end(), -1);
    (*si) = slider;
    (*sni).second = slider;
    
    return si - _sliders.begin();
  }

  // This is the first slider with this name.
  int new_index = (int)_sliders.size();
  _sliders.push_back(slider);
  _sliders_by_name[slider->get_name()] = slider;

  return new_index;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SliderTable::
write(ostream &out) const {
  for (size_t i = 0; i < _sliders.size(); ++i) {
    out << i << ". " << *_sliders[i] << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::do_register
//       Access: Private
//  Description: Called internally when the table is registered.
////////////////////////////////////////////////////////////////////
void SliderTable::
do_register() {
  nassertv(!_is_registered);

  Sliders::iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    const VertexSlider *slider = (*si);
    bool inserted = ((VertexSlider *)slider)->_tables.insert(this).second;
    nassertv(inserted);
  }
  _is_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::do_unregister
//       Access: Private
//  Description: Called internally when the table is unregistered
//               (i.e. right before destruction).
////////////////////////////////////////////////////////////////////
void SliderTable::
do_unregister() {
  nassertv(_is_registered);

  Sliders::iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    const VertexSlider *slider = (*si);
    ((VertexSlider *)slider)->_tables.erase(this);
  }
  _is_registered = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               SliderTable.
////////////////////////////////////////////////////////////////////
void SliderTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SliderTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_sliders.size());
  Sliders::const_iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    manager->write_pointer(dg, (*si)->get_name());
    manager->write_pointer(dg, (*si));
  }

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int SliderTable::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  Sliders::iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    CPT(InternalName) name = DCAST(InternalName, p_list[pi++]);
    PT(VertexSlider) slider = DCAST(VertexSlider, p_list[pi++]);

    (*si) = slider;

    bool inserted = _sliders_by_name.insert(SlidersByName::value_type(name, slider.p())).second;
    nassertr(inserted, pi);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type SliderTable is encountered
//               in the Bam file.  It should create the SliderTable
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *SliderTable::
make_from_bam(const FactoryParams &params) {
  SliderTable *object = new SliderTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new SliderTable.
////////////////////////////////////////////////////////////////////
void SliderTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  size_t num_sliders = scan.get_uint16();
  _sliders.reserve(num_sliders);
  for (size_t i = 0; i < num_sliders; ++i) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    _sliders.push_back(NULL);
  }

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *SliderTable::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SliderTable::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

////////////////////////////////////////////////////////////////////
//     Function: SliderTable::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new SliderTable.
////////////////////////////////////////////////////////////////////
void SliderTable::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _modified = VertexTransform::get_next_modified();
}

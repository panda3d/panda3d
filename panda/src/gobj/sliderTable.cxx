/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sliderTable.cxx
 * @author drose
 * @date 2005-03-28
 */

#include "sliderTable.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "vertexTransform.h"

SparseArray SliderTable::_empty_array;
TypeHandle SliderTable::_type_handle;

/**
 *
 */
SliderTable::
SliderTable() :
  _is_registered(false)
{
}

/**
 *
 */
SliderTable::
SliderTable(const SliderTable &copy) :
  _is_registered(false),
  _sliders(copy._sliders),
  _sliders_by_name(copy._sliders_by_name)
{
}

/**
 *
 */
void SliderTable::
operator = (const SliderTable &copy) {
  nassertv(!_is_registered);
  _sliders = copy._sliders;
  _sliders_by_name = copy._sliders_by_name;
}

/**
 *
 */
SliderTable::
~SliderTable() {
  if (_is_registered) {
    do_unregister();
  }
}

/**
 * Replaces the nth slider.  Only valid for unregistered tables.
 */
void SliderTable::
set_slider(size_t n, const VertexSlider *slider) {
  nassertv(!_is_registered);
  nassertv(n < _sliders.size());

  if (_sliders[n]._slider->get_name() != slider->get_name()) {
    _sliders_by_name[_sliders[n]._slider->get_name()].clear_bit(n);
    _sliders_by_name[slider->get_name()].set_bit(n);
  }

  _sliders[n]._slider = slider;
}

/**
 * Replaces the rows affected by the nth slider.  Only valid for unregistered
 * tables.
 */
void SliderTable::
set_slider_rows(size_t n, const SparseArray &rows) {
  // We don't actually enforce the registration requirement, since gee, it
  // doesn't actually matter here; and the GeomVertexData needs to be able to
  // change the SparseArrays in the bam reader.  nassertv(!_is_registered);
  nassertv(n < _sliders.size());

  _sliders[n]._rows = rows;
}

/**
 * Removes the nth slider.  Only valid for unregistered tables.
 */
void SliderTable::
remove_slider(size_t n) {
  nassertv(!_is_registered);
  nassertv(n < _sliders.size());

  _sliders_by_name[_sliders[n]._slider->get_name()].clear_bit(n);
  _sliders.erase(_sliders.begin() + n);
}

/**
 * Adds a new slider to the table, and returns the index number of the new
 * slider.  Only valid for unregistered tables.
 */
size_t SliderTable::
add_slider(const VertexSlider *slider, const SparseArray &rows) {
  nassertr(!_is_registered, 0);

  size_t new_index = _sliders.size();

  SliderDef slider_def;
  slider_def._slider = slider;
  slider_def._rows = rows;
  _sliders.push_back(slider_def);
  _sliders_by_name[slider->get_name()].set_bit(new_index);

  return new_index;
}

/**
 *
 */
void SliderTable::
write(std::ostream &out) const {
  for (size_t i = 0; i < _sliders.size(); ++i) {
    out << i << ". " << *_sliders[i]._slider << " "
        << _sliders[i]._rows << "\n";
  }
}

/**
 * Called internally when the table is registered.
 */
void SliderTable::
do_register() {
  nassertv(!_is_registered);

  Sliders::iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    const VertexSlider *slider = (*si)._slider;
    bool inserted = ((VertexSlider *)slider)->_tables.insert(this).second;
    nassertv(inserted);
  }
  _is_registered = true;
}

/**
 * Called internally when the table is unregistered (i.e.  right before
 * destruction).
 */
void SliderTable::
do_unregister() {
  nassertv(_is_registered);

  Sliders::iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    const VertexSlider *slider = (*si)._slider;
    ((VertexSlider *)slider)->_tables.erase(this);
  }
  _is_registered = false;
}

/**
 * Tells the BamReader how to create objects of type SliderTable.
 */
void SliderTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SliderTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_sliders.size());
  Sliders::const_iterator si;
  for (si = _sliders.begin(); si != _sliders.end(); ++si) {
    manager->write_pointer(dg, (*si)._slider->get_name());
    manager->write_pointer(dg, (*si)._slider);
    (*si)._rows.write_datagram(manager, dg);
  }

  manager->write_cdata(dg, _cycler);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int SliderTable::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritableReferenceCount::complete_pointers(p_list, manager);

  for (size_t n = 0; n < _sliders.size(); ++n) {
    CPT(InternalName) name = DCAST(InternalName, p_list[pi++]);
    PT(VertexSlider) slider = DCAST(VertexSlider, p_list[pi++]);

    _sliders[n]._slider = slider;
    _sliders_by_name[name].set_bit(n);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type SliderTable is encountered in the Bam file.  It should create the
 * SliderTable and extract its information from the file.
 */
TypedWritable *SliderTable::
make_from_bam(const FactoryParams &params) {
  SliderTable *object = new SliderTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SliderTable.
 */
void SliderTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  size_t num_sliders = scan.get_uint16();
  _sliders.reserve(num_sliders);
  for (size_t i = 0; i < num_sliders; ++i) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    _sliders.push_back(SliderDef());
    if (manager->get_file_minor_ver() >= 7) {
      _sliders[i]._rows.read_datagram(scan, manager);
    } else {
      // In this case, for bam files prior to 6.7, we must define the
      // SparseArray with the full number of vertices.  This is done in
      // GeomVertexData::complete_pointers().
    }
  }

  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
CycleData *SliderTable::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SliderTable::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SliderTable.
 */
void SliderTable::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  Thread *current_thread = Thread::get_current_thread();
  _modified = VertexTransform::get_next_modified(current_thread);
}

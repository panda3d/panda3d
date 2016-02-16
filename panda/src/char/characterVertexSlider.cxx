/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterVertexSlider.cxx
 * @author drose
 * @date 2005-03-28
 */

#include "characterVertexSlider.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CharacterVertexSlider::_type_handle;

/**
 * Constructs an invalid object; used only by the bam loader.
 */
CharacterVertexSlider::
CharacterVertexSlider() :
  VertexSlider(InternalName::get_root())
{
}

/**
 * Constructs a new object that converts vertices from the indicated joint's
 * coordinate space, into the other indicated joint's space.
 */
CharacterVertexSlider::
CharacterVertexSlider(CharacterSlider *char_slider) :
  VertexSlider(InternalName::make(char_slider->get_name())),
  _char_slider(char_slider)
{
  // Tell the char_slider that we need to be informed when it moves.
  _char_slider->_vertex_sliders.insert(this);
}

/**

 */
CharacterVertexSlider::
~CharacterVertexSlider() {
  // Tell the char_slider to stop informing us about its motion.
  _char_slider->_vertex_sliders.erase(this);
}

/**
 * Returns the current slider value.
 */
PN_stdfloat CharacterVertexSlider::
get_slider() const {
  return _char_slider->_value;
}

/**
 * Tells the BamReader how to create objects of type CharacterVertexSlider.
 */
void CharacterVertexSlider::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a Bam
 * file.
 */
void CharacterVertexSlider::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexSlider::write_datagram(manager, dg);

  manager->write_pointer(dg, _char_slider);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer() was
 * called in fillin(). Returns the number of pointers processed.
 */
int CharacterVertexSlider::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = VertexSlider::complete_pointers(p_list, manager);

  _char_slider = DCAST(CharacterSlider, p_list[pi++]);
  _char_slider->_vertex_sliders.insert(this);
  _name = InternalName::make(_char_slider->get_name());

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of type
 * CharacterVertexSlider is encountered in the Bam file.  It should create the
 * CharacterVertexSlider and extract its information from the file.
 */
TypedWritable *CharacterVertexSlider::
make_from_bam(const FactoryParams &params) {
  CharacterVertexSlider *object = new CharacterVertexSlider;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CharacterVertexSlider.
 */
void CharacterVertexSlider::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexSlider::fillin(scan, manager);

  manager->read_pointer(scan);
}

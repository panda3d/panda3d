/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file userVertexSlider.cxx
 * @author drose
 * @date 2005-03-28
 */

#include "userVertexSlider.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle UserVertexSlider::_type_handle;

/**
 *
 */
UserVertexSlider::
UserVertexSlider(const std::string &name) :
  VertexSlider(InternalName::make(name))
{
}

/**
 *
 */
UserVertexSlider::
UserVertexSlider(const InternalName *name) :
  VertexSlider(name)
{
}

/**
 * Returns the current slider value.
 */
PN_stdfloat UserVertexSlider::
get_slider() const {
  CDReader cdata(_cycler);
  return cdata->_slider;
}

/**
 *
 */
CycleData *UserVertexSlider::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Tells the BamReader how to create objects of type UserVertexSlider.
 */
void UserVertexSlider::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void UserVertexSlider::
write_datagram(BamWriter *manager, Datagram &dg) {
  VertexSlider::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type UserVertexSlider is encountered in the Bam file.  It should create the
 * UserVertexSlider and extract its information from the file.
 */
TypedWritable *UserVertexSlider::
make_from_bam(const FactoryParams &params) {
  UserVertexSlider *object = new UserVertexSlider("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new UserVertexSlider.
 */
void UserVertexSlider::
fillin(DatagramIterator &scan, BamReader *manager) {
  VertexSlider::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void UserVertexSlider::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_stdfloat(_slider);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new UserVertexSlider.
 */
void UserVertexSlider::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _slider = scan.get_stdfloat();
}

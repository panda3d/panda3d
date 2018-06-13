/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rectangleLight.cxx
 * @author rdb
 * @date 2016-12-19
 */

#include "rectangleLight.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle RectangleLight::_type_handle;

/**
 *
 */
CycleData *RectangleLight::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RectangleLight::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_stdfloat(_max_distance);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void RectangleLight::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _max_distance = scan.get_stdfloat();
}

/**
 *
 */
RectangleLight::
RectangleLight(const std::string &name) :
  LightLensNode(name)
{
}

/**
 * Do not call the copy constructor directly; instead, use make_copy() or
 * copy_subgraph() to make a copy of a node.
 */
RectangleLight::
RectangleLight(const RectangleLight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *RectangleLight::
make_copy() const {
  return new RectangleLight(*this);
}

/**
 *
 */
void RectangleLight::
write(std::ostream &out, int indent_level) const {
  LightLensNode::write(out, indent_level);
  indent(out, indent_level) << *this << "\n";
}

/**
 * Returns the relative priority associated with all lights of this class.
 * This priority is used to order lights whose instance priority
 * (get_priority()) is the same--the idea is that other things being equal,
 * AmbientLights (for instance) are less important than DirectionalLights.
 */
int RectangleLight::
get_class_priority() const {
  return (int)CP_area_priority;
}

/**
 *
 */
void RectangleLight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
}

/**
 * Tells the BamReader how to create objects of type RectangleLight.
 */
void RectangleLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RectangleLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type RectangleLight is encountered in the Bam file.  It should create the
 * RectangleLight and extract its information from the file.
 */
TypedWritable *RectangleLight::
make_from_bam(const FactoryParams &params) {
  RectangleLight *node = new RectangleLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RectangleLight.
 */
void RectangleLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

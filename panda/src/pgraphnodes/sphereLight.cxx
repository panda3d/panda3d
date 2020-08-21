/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sphereLight.cxx
 * @author rdb
 * @date 2016-04-15
 */

#include "sphereLight.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle SphereLight::_type_handle;

/**
 *
 */
CycleData *SphereLight::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SphereLight::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_stdfloat(_radius);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void SphereLight::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _radius = scan.get_stdfloat();
}

/**
 *
 */
SphereLight::
SphereLight(const std::string &name) :
  PointLight(name)
{
}

/**
 * Do not call the copy constructor directly; instead, use make_copy() or
 * copy_subgraph() to make a copy of a node.
 */
SphereLight::
SphereLight(const SphereLight &copy) :
  PointLight(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *SphereLight::
make_copy() const {
  return new SphereLight(*this);
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void SphereLight::
xform(const LMatrix4 &mat) {
  PointLight::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_radius = mat.xform_vec(LVector3(0, 0, cdata->_radius)).length();
  mark_viz_stale();
}

/**
 *
 */
void SphereLight::
write(std::ostream &out, int indent_level) const {
  PointLight::write(out, indent_level);
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "radius " << get_radius() << "\n";
}

/**
 * Tells the BamReader how to create objects of type SphereLight.
 */
void SphereLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void SphereLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  PointLight::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type SphereLight is encountered in the Bam file.  It should create the
 * SphereLight and extract its information from the file.
 */
TypedWritable *SphereLight::
make_from_bam(const FactoryParams &params) {
  SphereLight *node = new SphereLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SphereLight.
 */
void SphereLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  PointLight::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

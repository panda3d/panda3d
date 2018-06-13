/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file directionalLight.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "directionalLight.h"
#include "orthographicLens.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DirectionalLight::_type_handle;

/**
 *
 */
CycleData *DirectionalLight::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void DirectionalLight::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _specular_color.write_datagram(dg);
  _point.write_datagram(dg);
  _direction.write_datagram(dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void DirectionalLight::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _specular_color.read_datagram(scan);
  _point.read_datagram(scan);
  _direction.read_datagram(scan);
}

/**
 *
 */
DirectionalLight::
DirectionalLight(const std::string &name) :
  LightLensNode(name, new OrthographicLens()) {
  _lenses[0]._lens->set_interocular_distance(0);
}

/**
 * Do not call the copy constructor directly; instead, use make_copy() or
 * copy_subgraph() to make a copy of a node.
 */
DirectionalLight::
DirectionalLight(const DirectionalLight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *DirectionalLight::
make_copy() const {
  return new DirectionalLight(*this);
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void DirectionalLight::
xform(const LMatrix4 &mat) {
  LightLensNode::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_point = cdata->_point * mat;
  cdata->_direction = cdata->_direction * mat;
  mark_viz_stale();
}

/**
 *
 */
void DirectionalLight::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
  if (_has_specular_color) {
    indent(out, indent_level + 2)
      << "specular color " << get_specular_color() << "\n";
  }
  indent(out, indent_level + 2)
    << "direction " << get_direction() << "\n";
}

/**
 * Computes the vector from a particular vertex to this light.  The exact
 * vector depends on the type of light (e.g.  point lights return a different
 * result than directional lights).
 *
 * The input parameters are the vertex position in question, expressed in
 * object space, and the matrix which converts from light space to object
 * space.  The result is expressed in object space.
 *
 * The return value is true if the result is successful, or false if it cannot
 * be computed (e.g.  for an ambient light).
 */
bool DirectionalLight::
get_vector_to_light(LVector3 &result, const LPoint3 &,
                    const LMatrix4 &to_object_space) {
  CDReader cdata(_cycler);
  result = -(cdata->_direction * to_object_space);

  return true;
}

/**
 * Returns the relative priority associated with all lights of this class.
 * This priority is used to order lights whose instance priority
 * (get_priority()) is the same--the idea is that other things being equal,
 * AmbientLights (for instance) are less important than DirectionalLights.
 */
int DirectionalLight::
get_class_priority() const {
  return (int)CP_directional_priority;
}

/**
 *
 */
void DirectionalLight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

/**
 * Tells the BamReader how to create objects of type DirectionalLight.
 */
void DirectionalLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void DirectionalLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  if (manager->get_file_minor_ver() >= 39) {
    dg.add_bool(_has_specular_color);
  }
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type DirectionalLight is encountered in the Bam file.  It should create the
 * DirectionalLight and extract its information from the file.
 */
TypedWritable *DirectionalLight::
make_from_bam(const FactoryParams &params) {
  DirectionalLight *node = new DirectionalLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new DirectionalLight.
 */
void DirectionalLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);

  if (manager->get_file_minor_ver() >= 39) {
    _has_specular_color = scan.get_bool();
  } else {
    _has_specular_color = true;
  }

  manager->read_cdata(scan, _cycler);
}

// Filename: directionalLight.cxx
// Created by:  mike (09Jan97)
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

#include "directionalLight.h"
#include "orthographicLens.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DirectionalLight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *DirectionalLight::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void DirectionalLight::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _specular_color.write_datagram(dg);
  _point.write_datagram(dg);
  _direction.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Light.
////////////////////////////////////////////////////////////////////
void DirectionalLight::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _specular_color.read_datagram(scan);
  _point.read_datagram(scan);
  _direction.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DirectionalLight::
DirectionalLight(const string &name) : 
  LightLensNode(name, new OrthographicLens())
{
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::Copy Constructor
//       Access: Protected
//  Description: Do not call the copy constructor directly; instead,
//               use make_copy() or copy_subgraph() to make a copy of
//               a node.
////////////////////////////////////////////////////////////////////
DirectionalLight::
DirectionalLight(const DirectionalLight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *DirectionalLight::
make_copy() const {
  return new DirectionalLight(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void DirectionalLight::
xform(const LMatrix4 &mat) {
  LightLensNode::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_point = cdata->_point * mat;
  cdata->_direction = cdata->_direction * mat;
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DirectionalLight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
  indent(out, indent_level + 2)
    << "specular color " << get_specular_color() << "\n";
  indent(out, indent_level + 2)
    << "direction " << get_direction() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::get_vector_to_light
//       Access: Public, Virtual
//  Description: Computes the vector from a particular vertex to this
//               light.  The exact vector depends on the type of light
//               (e.g. point lights return a different result than
//               directional lights).
//
//               The input parameters are the vertex position in
//               question, expressed in object space, and the matrix
//               which converts from light space to object space.  The
//               result is expressed in object space.
//
//               The return value is true if the result is successful,
//               or false if it cannot be computed (e.g. for an
//               ambient light).
////////////////////////////////////////////////////////////////////
bool DirectionalLight::
get_vector_to_light(LVector3 &result, const LPoint3 &, 
                    const LMatrix4 &to_object_space) {
  CDReader cdata(_cycler);
  result = -(cdata->_direction * to_object_space);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::get_class_priority
//       Access: Published, Virtual
//  Description: Returns the relative priority associated with all
//               lights of this class.  This priority is used to order
//               lights whose instance priority (get_priority()) is
//               the same--the idea is that other things being equal,
//               AmbientLights (for instance) are less important than
//               DirectionalLights.
////////////////////////////////////////////////////////////////////
int DirectionalLight::
get_class_priority() const {
  return (int)CP_directional_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::bind
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DirectionalLight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               DirectionalLight.
////////////////////////////////////////////////////////////////////
void DirectionalLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void DirectionalLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type DirectionalLight is encountered
//               in the Bam file.  It should create the DirectionalLight
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *DirectionalLight::
make_from_bam(const FactoryParams &params) {
  DirectionalLight *node = new DirectionalLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new DirectionalLight.
////////////////////////////////////////////////////////////////////
void DirectionalLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

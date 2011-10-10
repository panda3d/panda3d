// Filename: pointLight.cxx
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

#include "pointLight.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle PointLight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PointLight::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PointLight::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PointLight::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _specular_color.write_datagram(dg);
  _attenuation.write_datagram(dg);
  _point.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Light.
////////////////////////////////////////////////////////////////////
void PointLight::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _specular_color.read_datagram(scan);
  _attenuation.read_datagram(scan);
  _point.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PointLight::
PointLight(const string &name) : 
  LightLensNode(name) 
{
  PT(Lens) lens;
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(1, 0, 0, 0, 0, 1);
  set_lens(0, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(-1, 0, 0, 0, 0, 1);
  set_lens(1, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, 1, 0, 0, 0, 1);
  set_lens(2, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, -1, 0, 0, 0, 1);
  set_lens(3, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, 0, 1, 0, 0, 1);
  set_lens(4, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, 0, -1, 0, 0, 1);
  set_lens(5, lens);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::Copy Constructor
//       Access: Protected
//  Description: Do not call the copy constructor directly; instead,
//               use make_copy() or copy_subgraph() to make a copy of
//               a node.
////////////////////////////////////////////////////////////////////
PointLight::
PointLight(const PointLight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
  PT(Lens) lens;
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(1, 0, 0, 0, 0, 1);
  set_lens(0, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(-1, 0, 0, 0, 0, 1);
  set_lens(1, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, 1, 0, 0, 0, 1);
  set_lens(2, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, -1, 0, 0, 0, 1);
  set_lens(3, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, 0, 1, 0, 0, 1);
  set_lens(4, lens);
  lens = new PerspectiveLens(90, 90);
  lens->set_view_vector(0, 0, -1, 0, 0, 1);
  set_lens(5, lens);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *PointLight::
make_copy() const {
  return new PointLight(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PointLight::
xform(const LMatrix4 &mat) {
  LightLensNode::xform(mat);
  CDWriter cdata(_cycler);
  cdata->_point = cdata->_point * mat;
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void PointLight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
  indent(out, indent_level + 2)
    << "specular color " << get_specular_color() << "\n";
  indent(out, indent_level + 2)
    << "attenuation " << get_attenuation() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::get_vector_to_light
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
bool PointLight::
get_vector_to_light(LVector3 &result, const LPoint3 &from_object_point, 
                    const LMatrix4 &to_object_space) {
  CDReader cdata(_cycler);
  LPoint3 point = cdata->_point * to_object_space;

  result = point - from_object_point;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::get_class_priority
//       Access: Published, Virtual
//  Description: Returns the relative priority associated with all
//               lights of this class.  This priority is used to order
//               lights whose instance priority (get_priority()) is
//               the same--the idea is that other things being equal,
//               AmbientLights (for instance) are less important than
//               DirectionalLights.
////////////////////////////////////////////////////////////////////
int PointLight::
get_class_priority() const {
  return (int)CP_point_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::bind
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void PointLight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               PointLight.
////////////////////////////////////////////////////////////////////
void PointLight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PointLight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type PointLight is encountered
//               in the Bam file.  It should create the PointLight
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PointLight::
make_from_bam(const FactoryParams &params) {
  PointLight *node = new PointLight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PointLight.
////////////////////////////////////////////////////////////////////
void PointLight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

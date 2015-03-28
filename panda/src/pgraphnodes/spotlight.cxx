// Filename: spotlight.cxx
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

#include "spotlight.h"
#include "graphicsStateGuardianBase.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "colorAttrib.h"
#include "texture.h"
#include "config_pgraph.h"
#include "pnmImage.h"

TypeHandle Spotlight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *Spotlight::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Spotlight::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  dg.add_stdfloat(_exponent);
  _specular_color.write_datagram(dg);
  _attenuation.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Light.
////////////////////////////////////////////////////////////////////
void Spotlight::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _exponent = scan.get_stdfloat();
  _specular_color.read_datagram(scan);
  _attenuation.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Spotlight::
Spotlight(const string &name) : 
  LightLensNode(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::Copy Constructor
//       Access: Protected
//  Description: Do not call the copy constructor directly; instead,
//               use make_copy() or copy_subgraph() to make a copy of
//               a node.
////////////////////////////////////////////////////////////////////
Spotlight::
Spotlight(const Spotlight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *Spotlight::
make_copy() const {
  return new Spotlight(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void Spotlight::
xform(const LMatrix4 &mat) {
  LightLensNode::xform(mat); 
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Spotlight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
  indent(out, indent_level + 2)
    << "specular color " << get_specular_color() << "\n";
  indent(out, indent_level + 2)
    << "attenuation " << get_attenuation() << "\n";
  indent(out, indent_level + 2)
    << "exponent " << get_exponent() << "\n";

  Lens *lens = get_lens();
  if (lens != (Lens *)NULL) {
    lens->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::get_vector_to_light
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
bool Spotlight::
get_vector_to_light(LVector3 &result, const LPoint3 &from_object_point, 
                    const LMatrix4 &to_object_space) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::make_spot
//       Access: Published, Static
//  Description: Returns a newly-generated Texture that renders a
//               circular spot image as might be cast from the
//               spotlight.  This may be projected onto target
//               geometry (for instance, via
//               NodePath::project_texture()) instead of actually
//               enabling the light itself, as a cheesy way to make a
//               high-resolution spot appear on the geometry.
//
//               pixel_width specifies the height and width of the new
//               texture in pixels, full_radius is a value in the
//               range 0..1 that indicates the relative size of the
//               fully bright center spot, and fg and bg are the
//               colors of the interior and exterior of the spot,
//               respectively.
////////////////////////////////////////////////////////////////////
PT(Texture) Spotlight::
make_spot(int pixel_width, PN_stdfloat full_radius, LColor &fg, LColor &bg) {
  int num_channels;
  if (fg[0] == fg[1] && fg[1] == fg[2] &&
      bg[0] == bg[1] && bg[1] == bg[2]) {
    // grayscale
    num_channels = 1;
  } else {
    // color
    num_channels = 3;
  }
  if (fg[3] != 1.0f || bg[3] != 1.0f) {
    // with alpha.
    ++num_channels;
  }
  PNMImage image(pixel_width, pixel_width, num_channels);
  image.render_spot(LCAST(float, fg), LCAST(float, bg), full_radius, 1.0);

  PT(Texture) tex = new Texture("spot");
  tex->load(image);
  tex->set_border_color(bg);
  tex->set_wrap_u(SamplerState::WM_border_color);
  tex->set_wrap_v(SamplerState::WM_border_color);

  tex->set_minfilter(SamplerState::FT_linear);
  tex->set_magfilter(SamplerState::FT_linear);

  return tex;
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::get_class_priority
//       Access: Published, Virtual
//  Description: Returns the relative priority associated with all
//               lights of this class.  This priority is used to order
//               lights whose instance priority (get_priority()) is
//               the same--the idea is that other things being equal,
//               AmbientLights (for instance) are less important than
//               DirectionalLights.
////////////////////////////////////////////////////////////////////
int Spotlight::
get_class_priority() const {
  return (int)CP_spot_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::bind
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Spotlight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the indicated GeomNode up with Geoms suitable
//               for rendering this light.
////////////////////////////////////////////////////////////////////
void Spotlight::
fill_viz_geom(GeomNode *viz_geom) {
  Lens *lens = get_lens();
  if (lens == (Lens *)NULL) {
    return;
  }
  
  PT(Geom) geom = lens->make_geometry();
  if (geom == (Geom *)NULL) {
    return;
  }

  viz_geom->add_geom(geom, get_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::get_viz_state
//       Access: Private
//  Description: Returns a RenderState for rendering the spotlight
//               visualization.
////////////////////////////////////////////////////////////////////
CPT(RenderState) Spotlight::
get_viz_state() {
  return RenderState::make
    (ColorAttrib::make_flat(get_color()));
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Spotlight.
////////////////////////////////////////////////////////////////////
void Spotlight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Spotlight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Spotlight is encountered
//               in the Bam file.  It should create the Spotlight
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *Spotlight::
make_from_bam(const FactoryParams &params) {
  Spotlight *node = new Spotlight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Spotlight.
////////////////////////////////////////////////////////////////////
void Spotlight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

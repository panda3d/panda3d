// Filename: spotlight.cxx
// Created by:  mike (09Jan97)
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
  dg.add_float32(_exponent);
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
  _exponent = scan.get_float32();
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
xform(const LMatrix4f &mat) {
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
//     Function: Spotlight::bind
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Spotlight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

////////////////////////////////////////////////////////////////////
//     Function: Spotlight::make_image
//       Access: Public
//  Description: Generates an image into the indicated texture of a
//               circle with a soft edge that corresponds to the
//               falloff of the spotlight.  This is intended to be
//               used to implement projected texture spotlights; the
//               image can be applied to geometry with UV's computed
//               appropriate to simulate the texture's region of
//               influence.
//
//               Returns true if the image is successfully generated,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool Spotlight::
make_image(Texture *texture, float radius) {
  if (texture == NULL) {
    pgraph_cat.error()
      << "Spotlight::make_image() - NULL texture" << endl;
    return false;
  }
  int size = min(texture->get_x_size(), texture->get_y_size());
  if (size == 0) {
    size = 64;
  }

  PNMImage image(size, size, 1);

  const Colorf &c4 = get_color();
  const RGBColord color(c4[0], c4[1], c4[2]);

  int half_width = (size - 2) / 2;
  float dXY = 1 / (float)half_width;
  float Y = dXY + dXY;
  float X, YY, dist_from_center, intensity;
  int tx, ty, tx2, ty2;

  for (int y = 0; y < half_width; y++, Y += dXY) {
    X = dXY * y + dXY;
    YY = Y * Y;
    ty = y + half_width;

    for (int x = y; x < half_width; x++, X += dXY) {
      dist_from_center = (float)sqrt(X * X + YY);
      float D = dist_from_center;
      if (D <= radius)
        intensity = 1.0f;
          else if (D < 1.0f)
        intensity = pow(cos((D-radius) /
                (1.0f-radius) * (MathNumbers::pi_f*0.5f)), get_exponent());
      else
        intensity = 0;

      tx = x + half_width;

      image.set_xel(tx, ty, color * intensity);
      image.set_xel(tx, size - ty - 1, color * intensity);
      image.set_xel(size - tx - 1, ty, color * intensity);
      image.set_xel(size - tx - 1, size - ty - 1, color * intensity);

      tx2 = ty; ty2 = tx;

      image.set_xel(tx2, ty2, color * intensity);
      image.set_xel(tx2, size - ty2 - 1, color * intensity);
      image.set_xel(size - tx2 - 1, ty2, color * intensity);
      image.set_xel(size - tx2 - 1, size - ty2 - 1, color * intensity);
    }
  }

  texture->load(image);

  return true;
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

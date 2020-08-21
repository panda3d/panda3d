/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spotlight.cxx
 * @author mike
 * @date 1997-01-09
 */

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

/**
 *
 */
CycleData *Spotlight::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Spotlight::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_stdfloat(_exponent);
  _specular_color.write_datagram(dg);
  _attenuation.write_datagram(dg);
  if (manager->get_file_minor_ver() >= 41) {
    dg.add_stdfloat(_max_distance);
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Light.
 */
void Spotlight::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _exponent = scan.get_stdfloat();
  _specular_color.read_datagram(scan);
  _attenuation.read_datagram(scan);
  if (manager->get_file_minor_ver() >= 41) {
    _max_distance = scan.get_stdfloat();
  }
}

/**
 *
 */
Spotlight::
Spotlight(const std::string &name) :
  LightLensNode(name) {
  _lenses[0]._lens->set_interocular_distance(0);
}

/**
 * Do not call the copy constructor directly; instead, use make_copy() or
 * copy_subgraph() to make a copy of a node.
 */
Spotlight::
Spotlight(const Spotlight &copy) :
  LightLensNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *Spotlight::
make_copy() const {
  return new Spotlight(*this);
}

/**
 * Transforms the contents of this PandaNode by the indicated matrix, if it
 * means anything to do so.  For most kinds of PandaNodes, this does nothing.
 */
void Spotlight::
xform(const LMatrix4 &mat) {
  LightLensNode::xform(mat);
  mark_viz_stale();
}

/**
 *
 */
void Spotlight::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2)
    << "color " << get_color() << "\n";
  if (_has_specular_color) {
    indent(out, indent_level + 2)
      << "specular color " << get_specular_color() << "\n";
  }
  indent(out, indent_level + 2)
    << "attenuation " << get_attenuation() << "\n";
  indent(out, indent_level + 2)
    << "exponent " << get_exponent() << "\n";

  if (!cinf(get_max_distance())) {
    indent(out, indent_level + 2)
      << "max distance " << get_max_distance() << "\n";
  }

  Lens *lens = get_lens();
  if (lens != nullptr) {
    lens->write(out, indent_level + 2);
  }
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
bool Spotlight::
get_vector_to_light(LVector3 &result, const LPoint3 &from_object_point,
                    const LMatrix4 &to_object_space) {
  return false;
}

/**
 * Returns a newly-generated Texture that renders a circular spot image as
 * might be cast from the spotlight.  This may be projected onto target
 * geometry (for instance, via NodePath::project_texture()) instead of
 * actually enabling the light itself, as a cheesy way to make a high-
 * resolution spot appear on the geometry.
 *
 * pixel_width specifies the height and width of the new texture in pixels,
 * full_radius is a value in the range 0..1 that indicates the relative size
 * of the fully bright center spot, and fg and bg are the colors of the
 * interior and exterior of the spot, respectively.
 */
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

/**
 * Returns the relative priority associated with all lights of this class.
 * This priority is used to order lights whose instance priority
 * (get_priority()) is the same--the idea is that other things being equal,
 * AmbientLights (for instance) are less important than DirectionalLights.
 */
int Spotlight::
get_class_priority() const {
  return (int)CP_spot_priority;
}

/**
 *
 */
void Spotlight::
bind(GraphicsStateGuardianBase *gsg, const NodePath &light, int light_id) {
  gsg->bind_light(this, light, light_id);
}

/**
 * Fills the indicated GeomNode up with Geoms suitable for rendering this
 * light.
 */
void Spotlight::
fill_viz_geom(GeomNode *viz_geom) {
  Lens *lens = get_lens();
  if (lens == nullptr) {
    return;
  }

  PT(Geom) geom = lens->make_geometry();
  if (geom == nullptr) {
    return;
  }

  viz_geom->add_geom(geom, get_viz_state());
}

/**
 * Returns a RenderState for rendering the spotlight visualization.
 */
CPT(RenderState) Spotlight::
get_viz_state() {
  return RenderState::make
    (ColorAttrib::make_flat(get_color()));
}

/**
 * Tells the BamReader how to create objects of type Spotlight.
 */
void Spotlight::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Spotlight::
write_datagram(BamWriter *manager, Datagram &dg) {
  LightLensNode::write_datagram(manager, dg);
  if (manager->get_file_minor_ver() >= 39) {
    dg.add_bool(_has_specular_color);
  }
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Spotlight is encountered in the Bam file.  It should create the
 * Spotlight and extract its information from the file.
 */
TypedWritable *Spotlight::
make_from_bam(const FactoryParams &params) {
  Spotlight *node = new Spotlight("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Spotlight.
 */
void Spotlight::
fillin(DatagramIterator &scan, BamReader *manager) {
  LightLensNode::fillin(scan, manager);

  if (manager->get_file_minor_ver() >= 39) {
    _has_specular_color = scan.get_bool();
  } else {
    _has_specular_color = true;
  }

  manager->read_cdata(scan, _cycler);
}

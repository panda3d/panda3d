/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file polylightNode.cxx
 * @author sshodhan
 * @date 2004-06-02
 */

#include "polylightNode.h"
#include "config_pgraph.h"
#include "nodePath.h"
#include "clockObject.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

#include <time.h>
#include <math.h>

TypeHandle PolylightNode::_type_handle;

/**
 * Use PolylightNode() to construct a new PolylightNode object.
 */
PolylightNode::
PolylightNode(const std::string &name) :
PandaNode(name)
{
  _enabled = true;
  set_pos(0,0,0);
  set_color(1,1,1);
  _radius = 50;
  set_attenuation(ALINEAR);
  _a0 = 1.0;
  _a1 = 0.1;
  _a2 = 0.01;
  _flickering = true;
  set_flicker_type(FRANDOM);
  _offset = -0.5;
  _scale = 0.1;
  _step_size = 0.1;
  _sin_freq = 2.0;
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *PolylightNode::
make_copy() const {
  return new PolylightNode(*this);
}

/**
 * Transforms the contents of this node by the indicated matrix, if it means
 * anything to do so.  For most kinds of nodes, this does nothing.
 */
void PolylightNode::
xform(const LMatrix4 &mat) {
  nassertv(!mat.is_nan());

  if (mat.almost_equal(LMatrix4::ident_mat())) {
    return;
  }

  _position = _position * mat;

  // This is a little cheesy and fails miserably in the presence of a non-
  // uniform scale.
  LVector3 radius_v = LVector3(_radius, 0.0f, 0.0f) * mat;
  _radius = length(radius_v);
  mark_internal_bounds_stale();
}

/**
 * If flickering is on, the do_poly_light function in PolylightNodeEffect will
 * compute this light's color based on the variations applied in this function
 * Variation can be sin or random Use offset, scale and step_size to tweak
 * Future addition: custom function variations to flicker
 */
LColor PolylightNode::flicker() const {
  PN_stdfloat r,g,b;

  PN_stdfloat variation= 0.0;
  LColor color = get_color();  //color = get_color_scenegraph();

  r = color[0];
  g = color[1];
  b = color[2];

  if (_flicker_type == FRANDOM) {
    // srand((int)ClockObject::get_global_clock()->get_frame_time());
    variation = (rand()%100);  // a value between 0-99
    variation /= 100.0;
    if (polylight_info)
      pgraph_cat.info() << "Random Variation: " << variation << std::endl;
  } else if (_flicker_type == FSIN) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    variation = sinf(now*_sin_freq);
    if (polylight_info)
      pgraph_cat.info() << "Variation: " << variation << std::endl;
    // can't use negative variation, so make it positive
    if (variation < 0.0)
      variation *= -1.0;
  } else if (_flicker_type == FCUSTOM) {
    // fixed point list of variation values coming soon... double index =
    // (ClockObject::get_global_clock()->get_frame_time() % len(fixed_points))
    // *  ClockObject::get_global_clock()->get_dt(); index *= _speed;
    /*if (!(int)index > len(fixed_points) {
        variation = _fixed_points[(int)index];
        variation += _offset;
        variation *= _scale;
      }*/
  }

  // variation += _offset; variation *= _scale;

  // printf("Variation: %f\n",variation);
  r += r * variation;
  g += g * variation;
  b += b * variation;

  /* CLAMPING
  if (fabs(r - color[0]) > 0.5 || fabs(g - color[1]) > 0.5 || fabs(b - color[2]) > 0.5) {
    r = color[0];
    g = color[1];
    b = color[2];
  }
  */
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug() << "Color R:" << r << "; G:" << g << "; B:" << b << std::endl;
  }
  return LColor(r,g,b,1.0);
}

/**
 * Returns a number less than zero if this PolylightNode sorts before the
 * other one, greater than zero if it sorts after, or zero if they are
 * equivalent.
 *
 * Two PolylightNodes are considered equivalent if they consist of exactly the
 * same properties Otherwise, they are different; different PolylightNodes
 * will be ranked in a consistent but undefined ordering; the ordering is
 * useful only for placing the PolylightNodes in a sorted container like an
 * STL set.
 */
int PolylightNode::
compare_to(const PolylightNode &other) const {

  if (_enabled != other._enabled) {
    return _enabled ? 1 :-1;
  }

  if (_radius != other._radius) {
    return _radius < other._radius ? -1 :1;
  }
  LVecBase3 position = get_pos();
  LVecBase3 other_position = other.get_pos();
  if (position != other_position) {
    return position < other_position ? -1 :1;
  }

  LColor color = get_color();
  LColor other_color = other.get_color();
  if (color != other_color) {
    return color < other_color ? -1 :1;
  }

  if (_attenuation_type != other._attenuation_type) {
    return _attenuation_type < other._attenuation_type ? -1 :1;
  }

  if (_a0 != other._a0) {
    return _a0 < other._a0 ? -1 :1;
  }

  if (_a1 != other._a1) {
    return _a1 < other._a1 ? -1 :1;
  }

  if (_a2 != other._a2) {
    return _a2 < other._a2 ? -1 :1;
  }

  if (_flickering != other._flickering) {
    return _flickering ? 1 :-1;
  }

  if (_flicker_type != other._flicker_type) {
    return _flicker_type < other._flicker_type ? -1 :1;
  }

  if (_offset != other._offset) {
    return _offset < other._offset ? -1 :1;
  }

  if (_scale != other._scale) {
    return _scale < other._scale ? -1 :1;
  }

  if (_step_size != other._step_size) {
    return _step_size < other._step_size ? -1 :1;
  }

  if (_sin_freq != other._sin_freq) {
    return _sin_freq < other._sin_freq ? -1 :1;
  }


  return 0;
}




/**
 * Tells the BamReader how to create objects of type PolylightNode
 */
void PolylightNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PolylightNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  _position.write_datagram(dg);
  LRGBColor color;
  color.set(_color[0], _color[1], _color[2]);
  color.write_datagram(dg);
  dg.add_stdfloat(_radius);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CompassEffect is encountered in the Bam file.  It should create the
 * CompassEffect and extract its information from the file.
 */
TypedWritable *PolylightNode::
make_from_bam(const FactoryParams &params) {
  PolylightNode *light = new PolylightNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  light->fillin(scan, manager);

  return light;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CompassEffect.
 */
void PolylightNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  _position.read_datagram(scan);
  LRGBColor color;
  color.read_datagram(scan);
  set_color(color[0], color[1], color[2]);
  _radius = scan.get_stdfloat();
}



/**
 *
 */
void PolylightNode::
output(std::ostream &out) const {
  out << get_type() << ":";
  // out << "Position: " << get_x() << " " << get_y() << " " << get_z() <<
  // "\n"; out << "Color: " << get_r() << " " << get_g() << " " << get_b() <<
  // "\n";
  out << "Radius: " << get_radius() << "\n";
}

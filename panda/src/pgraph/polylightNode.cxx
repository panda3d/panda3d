// Filename: PolylightNode.cxx
// Created by:  sshodhan (02Jun04)
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

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::Constructor
//       Access: Published
//  Description: Use PolylightNode() to construct a new
//               PolylightNode object.
////////////////////////////////////////////////////////////////////
PolylightNode::
PolylightNode(const string &name) :
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

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::Constructor
//       Access: Public
//  Description: If flickering is on, the do_poly_light function
//               in PolylightNodeEffect will compute this light's color
//               based on the variations applied in this function
//               Variation can be sin or random
//               Use offset, scale and step_size to tweak
//               Future addition: custom function variations to flicker
////////////////////////////////////////////////////////////////////
Colorf PolylightNode::flicker() const {
  float r,g,b;

  Colorf color;
  color = get_color_scenegraph();
  r = color[0];
  g = color[1];
  b = color[2];
  float variation= 0.0;
  
  if (_flicker_type == FRANDOM) {
    //srand((int)ClockObject::get_global_clock()->get_frame_time());
    variation = (rand()%100);// * ClockObject::get_global_clock()->get_dt();
    variation /= 100.0;
    //printf("Random Variation: %f\n",variation);
    variation += _offset;
    variation *= _scale;
  } else if (_flicker_type == FSIN) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    variation = sinf(now*_sin_freq);// * ClockObject::get_global_clock()->get_dt();
    //printf("Variation: %f\n",variation);
    variation += _offset;
    variation *= _scale;
  } else if (_flicker_type == FCUSTOM) {
    // fixed point list of variation values coming soon...
    //double index = (ClockObject::get_global_clock()->get_frame_time() % len(fixed_points)) *  ClockObject::get_global_clock()->get_dt();
    //index *= _speed;
    /*if (!(int)index > len(fixed_points) {
        variation = _fixed_points[(int)index];
        variation += _offset;
        variation *= _scale;
      }*/
  }
  //printf("Variation: %f\n",variation);
  r+=variation;
  g+=variation;
  b+=variation;
 
  /* CLAMPING
  if (fabs(r - color[0]) > 0.5 || fabs(g - color[1]) > 0.5 || fabs(b - color[2]) > 0.5) {
    r = color[0];
    g = color[1];
    b = color[2];
  }
  */
  //printf("Color R:%f G:%f B:%f\n",r,g,b);
  return Colorf(r,g,b,1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::compare_to
//       Access: Published
//  Description: Returns a number less than zero if this PolylightNode
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two PolylightNodes are considered equivalent if they
//               consist of exactly the same properties
//               Otherwise, they are different; different
//               PolylightNodes will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the PolylightNodes in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
int PolylightNode::
compare_to(const PolylightNode &other) const {
  
  if (_enabled != other._enabled) {
    return _enabled ? 1 :-1;
  }

  if (_radius != other._radius) {
    return _radius < other._radius ? -1 :1;
  }
  LVecBase3f position = get_pos();
  LVecBase3f other_position = other.get_pos();
  if (position != other_position) {
    return position < other_position ? -1 :1;
  }

  Colorf color = get_color();
  Colorf other_color = other.get_color();
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




////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               PolylightNode
////////////////////////////////////////////////////////////////////
void PolylightNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PolylightNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  LVecBase3f position;
  Colorf color;
  position = get_pos();
  color = get_color();
  dg.add_float32(position[0]);
  dg.add_float32(position[1]);
  dg.add_float32(position[2]);
  dg.add_float32(color[0]);
  dg.add_float32(color[1]);
  dg.add_float32(color[2]);
  dg.add_float32(_radius);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CompassEffect is encountered
//               in the Bam file.  It should create the CompassEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PolylightNode::
make_from_bam(const FactoryParams &params) {
  PolylightNode *light = new PolylightNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  light->fillin(scan, manager);

  return light;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CompassEffect.
////////////////////////////////////////////////////////////////////
void PolylightNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  LVecBase3f position;
  Colorf color;
  position[0] = scan.get_float32();
  position[1] = scan.get_float32();
  position[2] = scan.get_float32();
  set_pos(position[0],position[1],position[2]);
  color[0] = scan.get_float32();
  color[1] = scan.get_float32();
  color[2] = scan.get_float32();
  set_color(color[0], color[1], color[2]);
  _radius = scan.get_float32();
}



////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void PolylightNode::
output(ostream &out) const {
  out << get_type() << ":";
  //out << "Position: " << get_x() << " " << get_y() << " " << get_z() << "\n";
  //out << "Color: " << get_r() << " " << get_g() << " " << get_b() << "\n";
  out << "Radius: " << get_radius() << "\n";
}


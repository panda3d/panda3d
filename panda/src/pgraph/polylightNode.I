// Filename: PolylightNodeEffect.I
// Created by:  sshodhan (02Jun04)
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



////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::operator ==
//       Access: Published
//  Description: Returns true if the two lights are equivalent
//               that is, all their properties are same
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
operator == (const PolylightNode &other) const {
  return (compare_to(other) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::operator !=
//       Access: Published
//  Description: Returns true if the two lights are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
operator != (const PolylightNode &other) const {
  return (compare_to(other) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::operator <
//       Access: Published
//  Description: Returns true if this PolylightNode sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent PolylightNodes is consistent but undefined,
//               and is useful only for storing PolylightNodes in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
operator < (const PolylightNode &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::is_enabled
//       Access: Published
//  Description: Is this light is enabled/disabled?
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
is_enabled() const {
  return _enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::enable
//       Access: Published
//  Description: Enable this light
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
enable(){
  _enabled=true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::disable
//       Access: Published
//  Description: Disable this light
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
disable(){
  _enabled=false;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_pos
//       Access: Published
//  Description: Set this light's position
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_pos(const LPoint3 &position) {
  _position = position;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_pos
//       Access: Published
//  Description: Set this light's position
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z){
  _position[0]=x;
  _position[1]=y;
  _position[2]=z;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_pos
//       Access: Published
//  Description: Returns position as a LPoint3
////////////////////////////////////////////////////////////////////
INLINE LPoint3 PolylightNode::
get_pos() const {
  return _position;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_radius
//       Access: Published
//  Description: Set radius of the spherical light volume
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_radius(PN_stdfloat r){
  _radius=r;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_radius
//       Access: Published
//  Description: Get radius of the spherical light volume
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_radius() const {
  return _radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_attenuation
//       Access: Published
//  Description: Set ALINEAR or AQUADRATIC attenuation
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
set_attenuation(PolylightNode::Attenuation_Type type){
  nassertr(type == ALINEAR || type == AQUADRATIC,false);
  _attenuation_type=type;
  return true;
  
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_attenuation
//       Access: Published
//  Description: Get "linear" or "quadratic" attenuation type
////////////////////////////////////////////////////////////////////
INLINE PolylightNode::Attenuation_Type PolylightNode::
get_attenuation() const {
  return _attenuation_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_a0
//       Access: Published
//  Description: Set the quadratic attenuation factor a0
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_a0(PN_stdfloat a0){
  _a0=a0;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_a1
//       Access: Published
//  Description: Set the quadratic attenuation factor a1
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_a1(PN_stdfloat a1){
  _a1=a1;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_a2
//       Access: Published
//  Description: Set the quadratic attenuation factor a2
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_a2(PN_stdfloat a2){
  _a2=a2;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_a0
//       Access: Published
//  Description: Get the quadratic attenuation factor a0
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_a0() const {
  return _a0;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_a1
//       Access: Published
//  Description: Get the quadratic attenuation factor a1
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_a1() const {
  return _a1;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_a2
//       Access: Published
//  Description: Get the quadratic attenuation factor a2
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_a2() const {
  return _a2;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::flicker_on
//       Access: Published
//  Description: Set flickering to true so at every loop this light's
//               color is varied based on flicker_type
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
flicker_on(){
  _flickering=true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::flicker_off
//       Access: Published
//  Description: Turn flickering off
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
flicker_off(){
  _flickering=false;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::is_flickering
//       Access: Published
//  Description: Check is this light is flickering
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
is_flickering() const {
  return _flickering;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_flicker_type
//       Access: Published
//  Description: Flicker type can be FRANDOM or FSIN
//               At a later point there might be a FCUSTOM
//               Custom flicker will be a set of fix points recorded
//               by animating the light's intensity
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::
set_flicker_type(PolylightNode::Flicker_Type type){
  nassertr(type == FRANDOM || type == FSIN,false);
  
  _flicker_type=type;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_flicker_type
//       Access: Published
//  Description: Returns FRANDOM or FSIN
////////////////////////////////////////////////////////////////////
INLINE PolylightNode::Flicker_Type PolylightNode::
get_flicker_type() const {
  return _flicker_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_offset
//       Access: Published
//  Description: Set the offset value for the random and sin
//               flicker variations... used to tweak the flicker
//               This value is added to the variation
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_offset(PN_stdfloat offset){
  _offset=offset;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_offset
//       Access: Published
//  Description: Get the offset value for the random and sin
//               flicker variations
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_scale
//       Access: Published
//  Description: Set the scale value for the random and sin
//               flicker variations... used to tweak the flicker
//               This value is multiplied with the variation
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_scale(PN_stdfloat scale){
  _scale=scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_scale
//       Access: Published
//  Description: Get the scale value for the random and sin
//               flicker variations
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_scale() const {
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_step_size
//       Access: Published
//  Description: Set the step size for the sin function in flicker
//               This is the increment size for the value supplied
//               to the sin function
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_step_size(PN_stdfloat step){
  _step_size=step;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_step_size
//       Access: Published
//  Description: Get the step size for the sin function in flicker
//               This is the increment size for the value supplied
//               to the sin function
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_step_size() const {
  return _step_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_color
//       Access: Published
//  Description: Set the light's color... 
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_color(const LColor &color) {
  //PandaNode::set_attrib(ColorAttrib::make_flat(color));
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_color
//       Access: Published
//  Description: Set the light's color... 3 floats between 0 and 1
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b) {
  /*
  LColor color;
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = 1.0;
  PandaNode::set_attrib(ColorAttrib::make_flat(color));
  */
  _color[0] = r;
  _color[1] = g;
  _color[2] = b;
  _color[3] = 1.0;  
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_color
//       Access: Published
//  Description: Returns the light's color as LColor
////////////////////////////////////////////////////////////////////
INLINE LColor PolylightNode::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_color_scenegraph
//       Access: Published
//  Description: This differs from get_color in that when applying
//               the light color we need to make sure that a color
//               flattening external to the PolylightNode is not 
//               ignored.
////////////////////////////////////////////////////////////////////
INLINE LColor PolylightNode::
get_color_scenegraph() const {

  const RenderAttrib *attrib =
    PandaNode::get_attrib(ColorAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const ColorAttrib *ca = DCAST(ColorAttrib, attrib);
    if (ca->get_color_type() == ColorAttrib::T_flat) {
      return ca->get_color();
    }
  }
    
  return _color;
 
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_freq
//       Access: Published
//  Description: Set frequency of sin flicker
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::
set_freq(PN_stdfloat f) {
  _sin_freq=f;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_freq
//       Access: Published
//  Description: Get frequency of sin flicker
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat PolylightNode::
get_freq() const {
  return _sin_freq;
}

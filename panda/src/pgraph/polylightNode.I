// Filename: PolylightNodeEffect.I
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
INLINE bool PolylightNode::is_enabled() const {
  return _enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::enable
//       Access: Published
//  Description: Enable this light
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::enable(){
  _enabled=true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::disable
//       Access: Published
//  Description: Disable this light
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::disable(){
  _enabled=false;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_pos
//       Access: Published
//  Description: Set this light's position
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_pos(float x, float y, float z){
  LVecBase3f position;
  position[0]=x;
  position[1]=y;
  position[2]=z;
  PandaNode::set_transform(get_transform()->set_pos(position));
  PandaNode::reset_prev_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_pos
//       Access: Published
//  Description: Returns position as a LPoint3f
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f PolylightNode::get_pos() const {
  return PandaNode::get_transform()->get_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_radius
//       Access: Published
//  Description: Set radius of the spherical light volume
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_radius(float r){
  _radius=r;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_radius
//       Access: Published
//  Description: Get radius of the spherical light volume
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_radius() const {
  return _radius;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_attenuation
//       Access: Published
//  Description: Set "linear" or "quadratic" attenuation
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::set_attenuation(string type){
  nassertr(type == "linear" || type == "quadratic",false);
    
  _attenuation_type=type;
  return true;
  
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_attenuation
//       Access: Published
//  Description: Get "linear" or "quadratic" attenuation type
////////////////////////////////////////////////////////////////////
INLINE string PolylightNode::get_attenuation() const {
  return _attenuation_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_a0
//       Access: Published
//  Description: Set the quadratic attenuation factor a0
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_a0(float a0){
  _a0=a0;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_a1
//       Access: Published
//  Description: Set the quadratic attenuation factor a1
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_a1(float a1){
  _a1=a1;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_a2
//       Access: Published
//  Description: Set the quadratic attenuation factor a2
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_a2(float a2){
  _a2=a2;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_a0
//       Access: Published
//  Description: Get the quadratic attenuation factor a0
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_a0() const {
  return _a0;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_a1
//       Access: Published
//  Description: Get the quadratic attenuation factor a1
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_a1() const {
  return _a1;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_a2
//       Access: Published
//  Description: Get the quadratic attenuation factor a2
//               fd = 1 / ( a0 + a1*distance + a2*distance*distance)
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_a2() const {
  return _a2;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::flicker_on
//       Access: Published
//  Description: Set flickering to true so at every loop this light's
//               color is varied based on flicker_type
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::flicker_on(){
  _flickering=true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::flicker_off
//       Access: Published
//  Description: Turn flickering off
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::flicker_off(){
  _flickering=false;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::is_flickering
//       Access: Published
//  Description: Check is this light is flickering
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::is_flickering() const {
  return _flickering;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_flicker_type
//       Access: Published
//  Description: Flicker type can be "random" or "sin"
//               At a later point there might be a "custom" 
//               Custom flicker will be a set of fix points recorded
//               by animating the light's intensity
////////////////////////////////////////////////////////////////////
INLINE bool PolylightNode::set_flicker_type(string type){
  nassertr(type == "random" || type == "sin",false);
  
  
  _flicker_type=type;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_flicker_type
//       Access: Published
//  Description: Returns "random" or "sin"
////////////////////////////////////////////////////////////////////
INLINE string PolylightNode::get_flicker_type() const {
  return _flicker_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_offset
//       Access: Published
//  Description: Set the offset value for the random and sin
//               flicker variations... used to tweak the flicker
//               This value is added to the variation
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_offset(float offset){
  _offset=offset;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_offset
//       Access: Published
//  Description: Get the offset value for the random and sin
//               flicker variations
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_offset() const {
  return _offset;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_scale
//       Access: Published
//  Description: Set the scale value for the random and sin
//               flicker variations... used to tweak the flicker
//               This value is multiplied with the variation
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_scale(float scale){
  _scale=scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_scale
//       Access: Published
//  Description: Get the scale value for the random and sin
//               flicker variations
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_scale() const {
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_step_size
//       Access: Published
//  Description: Set the step size for the sin function in flicker
//               This is the increment size for the value supplied
//               to the sin function
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_step_size(float step){
  _step_size=step;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_step_size
//       Access: Published
//  Description: Get the step size for the sin function in flicker
//               This is the increment size for the value supplied
//               to the sin function
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_step_size() const {
  return _step_size;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_color
//       Access: Published
//  Description: Set the light's color... 3 floats between 0 and 1
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_color(float r, float g, float b) {
  Colorf color;
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = 1.0;
  PandaNode::set_attrib(ColorAttrib::make_flat(color));
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_color
//       Access: Published
//  Description: Returns the light's color as Colorf
////////////////////////////////////////////////////////////////////
INLINE Colorf PolylightNode::get_color() const {

  const RenderAttrib *attrib =
    PandaNode::get_attrib(ColorAttrib::get_class_type());
  if (attrib != (const RenderAttrib *)NULL) {
    const ColorAttrib *ca = DCAST(ColorAttrib, attrib);
    if (ca->get_color_type() == ColorAttrib::T_flat) {
      return ca->get_color();
    }
  }
    
  return Colorf(1.0f, 1.0f, 1.0f, 1.0f);
 
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_x
//       Access: Published
//  Description: Set light's x position
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_x(float x) {
  LPoint3f pos = get_pos();
  pos[0] = x;
  set_pos(pos[0],pos[1],pos[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_y
//       Access: Published
//  Description: Set light's y position
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_y(float y) {
  LPoint3f pos = get_pos();
  pos[1] = y;
  set_pos(pos[0],pos[1],pos[2]);
}
 
////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_z
//       Access: Published
//  Description: Set light's z position
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_z(float z) {
  LPoint3f pos = get_pos();
  pos[2] = z;
  set_pos(pos[0],pos[1],pos[2]);
}
/*
////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_r
//       Access: Published
//  Description: Set light's red between 0 and 1
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_r(float r) {
  _color[0]=r;
}
 
////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_g
//       Access: Published
//  Description: Set light's green between 0 and 1
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_g(float g) {
  _color[1]=g;
}
 
////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_b
//       Access: Published
//  Description: Set light's blue between 0 and 1
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_b(float b) {
  _color[2]=b;
}
*/
////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_x
//       Access: Published
//  Description: Get light's x position
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_x() const {
  return get_pos()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_y
//       Access: Published
//  Description: Get light's y position
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_y() const {
  return get_pos()[1];
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_z
//       Access: Published
//  Description: Get light's z position
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_z() const {
  return get_pos()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_r
//       Access: Published
//  Description: Get light's red color
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_r() const {
  return get_color()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_g
//       Access: Published
//  Description: Get light's green color
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_g() const {
  return get_color()[1];
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_b
//       Access: Published
//  Description: Get light's blue color
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_b() const {
  return get_color()[2];
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::set_freq
//       Access: Published
//  Description: Set frequency of sin flicker
////////////////////////////////////////////////////////////////////
INLINE void PolylightNode::set_freq(float f) {
  _sin_freq=f;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightNode::get_freq
//       Access: Published
//  Description: Get frequency of sin flicker
////////////////////////////////////////////////////////////////////
INLINE float PolylightNode::get_freq() const {
  return _sin_freq;
}
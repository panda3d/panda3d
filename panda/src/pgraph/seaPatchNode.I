// Filename: seaPatchNode.I
// Created by:  sshodhan (18Jun04)
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
//     Function: SeaPatchNode::operator ==
//       Access: Published
//  Description: Returns true if the two lights are equivalent
//               that is, all their properties are same
////////////////////////////////////////////////////////////////////
INLINE bool SeaPatchNode::
operator == (const SeaPatchNode &other) const {
  return (compare_to(other) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::operator !=
//       Access: Published
//  Description: Returns true if the two lights are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool SeaPatchNode::
operator != (const SeaPatchNode &other) const {
  return (compare_to(other) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::operator <
//       Access: Published
//  Description: Returns true if this SeaPatchNode sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent SeaPatchNodes is consistent but undefined,
//               and is useful only for storing SeaPatchNodes in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool SeaPatchNode::
operator < (const SeaPatchNode &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::is_enabled
//       Access: Published
//  Description: Is this light is enabled/disabled?
////////////////////////////////////////////////////////////////////
INLINE bool SeaPatchNode::
is_enabled() const {
  return _enabled;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::enable
//       Access: Published
//  Description: Enable this light
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
enable(){
  _enabled=true;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::disable
//       Access: Published
//  Description: Disable this light
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
disable(){
  _enabled=false;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_amp
//       Access: Published
//  Description: Set Wave Amplitude
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_amp(float amplitude) {
  _amplitude = amplitude;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_freq
//       Access: Published
//  Description: Set Wave Frequency
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_freq(float frequency) {
  _frequency = frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_amp
//       Access: Published
//  Description: Get Wave amplitude
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_amp() const {
  return _amplitude;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_freq
//       Access: Published
//  Description: Get Wave frequency
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_freq() const {
  return _frequency;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_passive_move
//       Access: Published
//  Description: This is the U and V speed at which textures will
//               slide in default mode. This is kind of like an 
//               idle cylce for the ocean water
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_passive_move(LVecBase2f UV) {
  _passive_move = UV;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_passive_move
//       Access: Published
//  Description: This is the U and V speed at which textures will
//               slide in default mode. This is kind of like an 
//               idle cylce for the ocean water
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f SeaPatchNode::
get_passive_move() const {
  return _passive_move;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_center
//       Access: Published
//  Description: Center for fade out effect
////////////////////////////////////////////////////////////////////
INLINE LPoint3f SeaPatchNode::
get_center() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_radius
//       Access: Published
//  Description: Radius for fade out effect
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_radius() const {
  return _radius;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_threshold
//       Access: Published
//  Description: Threshold for fade out effect
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_threshold() const {
  return _threshold;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_center
//       Access: Published
//  Description: Center for fade out effect
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_center(LPoint3f center) {
  _center = center;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_radius
//       Access: Published
//  Description: Radius for fade out effect
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_radius(float r) {
  _radius = r;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_threshold
//       Access: Published
//  Description: Radius for fade out effect
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_threshold(float t) {
  _threshold = t;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_noise_amp
//       Access: Published
//  Description: Set Noise Amplitude
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_noise_amp(float x) {
  _noise_amp = x;
} 

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_noise_amp
//       Access: Published
//  Description: Amplitude of noise
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_noise_amp() const {
  return _noise_amp;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::enable_noise_uv
//       Access: Published
//  Description: Apply noise to UVs
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
enable_noise_uv() {
  _noise_on_uv = true;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::disable_noise_uv
//       Access: Published
//  Description: Disable applying noise to UVs
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
disable_noise_uv() {
  _noise_on_uv = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::is_noise_on_uv
//       Access: Published
//  Description: Check if noise is on
////////////////////////////////////////////////////////////////////
INLINE bool SeaPatchNode::
is_noise_on_uv() const {
  return _noise_on_uv;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::set_noise_f
//       Access: Published
//  Description: Set Noise Frequency
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_noise_f(float f) {
  _noise_f = f;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::get_noise_f
//       Access: Published
//  Description: Get Noise Frequency
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_noise_f() {
  return _noise_f;
}

////////////////////////////////////////////////////////////////////
//     Function: SeaPatchNode::Weight
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
Weight(float t) {
  return (2.0*fabs(t)-3.0) * t*t + 1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Mod
//  Description: An implementation of modulo (%) that is correct for
//               negative numbers.
////////////////////////////////////////////////////////////////////
INLINE int SeaPatchNode::
Mod(int quotient, int divisor) {
  return (quotient >= 0) ? 
    (quotient % divisor) :
    (divisor-1 - ((-quotient-1) % divisor));
}

////////////////////////////////////////////////////////////////////
//     Function: set_xsize
//  Description: A scale factor for x values to noise function
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_xsize(float xs) {
  _xsize = xs;
}

////////////////////////////////////////////////////////////////////
//     Function: set_ysize
//  Description: A scale factor for y values to noise function
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_ysize(float ys) {
  _ysize = ys;
}

////////////////////////////////////////////////////////////////////
//     Function: get_xsize
//  Description: A scale factor for x values to noise function
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_xsize() const {
  return _xsize;
}

////////////////////////////////////////////////////////////////////
//     Function: get_ysize
//  Description: A scale factor for y values to noise function
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_ysize() const {
  return _ysize;
}


////////////////////////////////////////////////////////////////////
//     Function: set_u_scale
//  Description: U coordinate computation scale
//               We recompute UVs every frame based on world coords
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_u_scale(float us) {
  _u_scale = us;
}

////////////////////////////////////////////////////////////////////
//     Function: set_v_scale
//  Description: V coordinate computation scale
//               We recompute UVs every frame based on world coords
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_v_scale(float vs) {
  _v_scale = vs;
}

////////////////////////////////////////////////////////////////////
//     Function: get_u_scale
//  Description: U coordinate computation scale
//               We recompute UVs every frame based on world coords
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_u_scale() const {
  return _u_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: get_v_scale
//  Description: V coordinate computation scale
//               We recompute UVs every frame based on world coords
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_v_scale() const {
  return _v_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: get_height
//  Description: Get the height of any point in the grid
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_height(float x, float y) const {
   return ((sin(_wave_movement + (x / _wavelength[0]) ) 
    + sin(_wave_movement + (y / _wavelength[1]))) * _amplitude);
}


////////////////////////////////////////////////////////////////////
//     Function: get_normal
//  Description: return the normal to any point in the grid
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f SeaPatchNode::
get_normal(float x, float y) const {
   float h, h1, h2;
   h = get_height(x,y);
   h1 = get_height(x - 0.3, y + 0.2);
   h2 = get_height(x - 0.4, y - 0.1);
      
   LVecBase3f a,b,N,s,a1, b1;
   
   a[0] = x - 0.3;
   a[1] = y + 0.2;
   a[2] = h1;
   
   b[0] = x - 0.4;
   b[1] = y - 0.1;
   b[2] = h2;
   
   s[0] = x;
   s[1] = y;
   s[2] = h;
   
   a1 = a - s;
   b1 = b - s;
   
   N = cross(a1,b1);
   
   
   float length = (float)sqrt(N[0]*N[0] + N[1]*N[1] + N[2]*N[2]);
   
   if(length == 0.0) {
     length = 1.0;
   }
   
   N[0] /= length;
   N[1] /= length;
   N[2] /= length;
   
   return N;
}


////////////////////////////////////////////////////////////////////
//     Function: set_cscale
//  Description: Color scale for the vertex color... its just an 
//               additive term
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_cscale(float cs) {
  _color_scale = cs;
}

////////////////////////////////////////////////////////////////////
//     Function: set_ascale
//  Description: Alpha scale for the vertex color... its just an 
//               additive term
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_ascale(float as) {
  _alpha_scale = as;
}

////////////////////////////////////////////////////////////////////
//     Function: get_cscale
//  Description: Color scale for the vertex color... its just an 
//               additive term
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_cscale() const {
  return _color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: get_ascale
//  Description: Alpha scale for the vertex color... its just an 
//               additive term
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_ascale() const {
  return _alpha_scale;
}


////////////////////////////////////////////////////////////////////
//     Function: set_light_color
//  Description: Vertex color has an additive light component
//               Contributes 1/3 of the color
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_light_color(Colorf c) {
  _light_color = c;
}

////////////////////////////////////////////////////////////////////
//     Function: get_light_color
//  Description: Vertex color has an additive light component
//               Contributes 1/3 of the color
////////////////////////////////////////////////////////////////////
INLINE Colorf SeaPatchNode::
get_light_color() const {
  return _light_color;
}

////////////////////////////////////////////////////////////////////
//     Function: get_u_sin_scale
//  Description: Pass a sin wave through the UV scaling
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_u_sin_scale() const {
  return _u_sin_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: get_v_sin_scale
//  Description: Pass a sin wave through the UV scaling
////////////////////////////////////////////////////////////////////
INLINE float SeaPatchNode::
get_v_sin_scale() const {
  return _v_sin_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: set_u_sin_scale
//  Description: Pass a sin wave through the UV scaling
////////////////////////////////////////////////////////////////////
INLINE void SeaPatchNode::
set_u_sin_scale(float s) {
  _u_sin_scale = s;
}

////////////////////////////////////////////////////////////////////
//     Function: set_u_sin_scale
//  Description: Pass a sin wave through the UV scaling
//////////////////////////////////////////////////////////////////// 
INLINE void SeaPatchNode::
set_v_sin_scale(float s) {
  _v_sin_scale = s;
}
  
  
////////////////////////////////////////////////////////////////////
//     Function: get_wavelength
//  Description: Get the wavelength dividing factor
//////////////////////////////////////////////////////////////////// 
INLINE LVecBase2f SeaPatchNode::
get_wavelength() const {
  return _wavelength;
}

////////////////////////////////////////////////////////////////////
//     Function: set_wavelength
//  Description: Set the wavelength dividing factor
//////////////////////////////////////////////////////////////////// 
INLINE void SeaPatchNode::
set_wavelength(LVecBase2f w) {
  _wavelength = w;
}



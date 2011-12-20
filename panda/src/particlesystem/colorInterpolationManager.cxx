// Filename: colorInterpolationManager.cxx
// Created by:  joswilso (02Jun05)
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
#include "colorInterpolationManager.h"
#include "mathNumbers.h"

TypeHandle ColorInterpolationFunction::_type_handle;
TypeHandle ColorInterpolationFunctionConstant::_type_handle;
TypeHandle ColorInterpolationFunctionLinear::_type_handle;
TypeHandle ColorInterpolationFunctionStepwave::_type_handle;
TypeHandle ColorInterpolationFunctionSinusoid::_type_handle;

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunction::ColorInterpolationFunction
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunction::
ColorInterpolationFunction() {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunction::~ColorInterpolationFunction
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunction::
~ColorInterpolationFunction() {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::ColorInterpolationFunctionConstant
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionConstant::
ColorInterpolationFunctionConstant() :
  _c_a(1.0f,1.0f,1.0f,1.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::ColorInterpolationFunctionConstant
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionConstant::
ColorInterpolationFunctionConstant(const LColor &color_a) :
  _c_a(color_a) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::interpolate
//      Access : protected
// Description : Returns the color associated with this instance.
////////////////////////////////////////////////////////////////////

LColor ColorInterpolationFunctionConstant::
interpolate(const PN_stdfloat t) const {
  return _c_a;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::ColorInterpolationFunctionLinear
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionLinear::
ColorInterpolationFunctionLinear() :
  _c_b(1.0f,1.0f,1.0f,1.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::ColorInterpolationFunctionLinear
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionLinear::
ColorInterpolationFunctionLinear(const LColor &color_a, 
                                 const LColor &color_b) :
  ColorInterpolationFunctionConstant(color_a),
  _c_b(color_b) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::interpolate
//      Access : protected
// Description : Returns the linear mixture of A and B according to 't'.
////////////////////////////////////////////////////////////////////

LColor ColorInterpolationFunctionLinear::
interpolate(const PN_stdfloat t) const {
  return (1.0f-t)*_c_a + t*_c_b;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::ColorInterpolationFunctionStepwave
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionStepwave::
ColorInterpolationFunctionStepwave() :
  _w_a(0.5f),
  _w_b(0.5f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::ColorInterpolationFunctionStepwave
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionStepwave::
ColorInterpolationFunctionStepwave(const LColor &color_a,
                                   const LColor &color_b, 
                                   const PN_stdfloat width_a,
                                   const PN_stdfloat width_b) :
  ColorInterpolationFunctionLinear(color_a,color_b),
  _w_a(width_a),
  _w_b(width_b) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::interpolate
//      Access : protected
// Description : Returns either A or B.
////////////////////////////////////////////////////////////////////

LColor ColorInterpolationFunctionStepwave::
interpolate(const PN_stdfloat t) const { 
  if(fmodf(t,(_w_a+_w_b))<_w_a) {
      return _c_a;
  }
  return _c_b;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionSinusoid::ColorInterpolationFunctionSinusoid
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionSinusoid::
ColorInterpolationFunctionSinusoid() :
  _period(1.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionSinusoid::ColorInterpolationFunctionSinusoid
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionSinusoid::
ColorInterpolationFunctionSinusoid(const LColor &color_a, 
                                   const LColor &color_b, 
                                   const PN_stdfloat period) :
  ColorInterpolationFunctionLinear(color_a,color_b),
  _period(period) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionSinusoid::interpolate
//      Access : protected
// Description : Returns a sinusoidal blended color between A and B.
//               Period defines the time it will take to return to
//               A.
////////////////////////////////////////////////////////////////////

LColor ColorInterpolationFunctionSinusoid::
interpolate(const PN_stdfloat t) const {
  PN_stdfloat weight_a = (1.0f+cos(t*MathNumbers::pi_f*2.0f/_period))/2.0f;
  return (weight_a*_c_a)+((1.0f-weight_a)*_c_b);
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::ColorInterpolationSegment
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationSegment::
ColorInterpolationSegment(ColorInterpolationFunction* function,
                          const PN_stdfloat &time_begin,
                          const PN_stdfloat &time_end,
                          const bool is_modulated,
                          const int id) :
  _color_inter_func(function),
  _t_begin(time_begin),
  _t_end(time_end),
  _t_total(time_end-time_begin),
  _is_modulated(is_modulated),
  _enabled(true),
  _id(id) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::ColorInterpolationSegment
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationSegment::
ColorInterpolationSegment(const ColorInterpolationSegment &copy) :
  _color_inter_func(copy._color_inter_func),
  _t_begin(copy._t_begin),
  _t_end(copy._t_end),
  _t_total(copy._t_total),
  _is_modulated(copy._is_modulated),
  _enabled(copy._enabled),
  _id(copy._id) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::~ColorInterpolationSegment
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

ColorInterpolationSegment::
~ColorInterpolationSegment() {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::interpolateColor
//      Access : public
// Description : Returns the interpolated color according to the
//               segment's function and start and end times.  't' is
//               a value in [0-1] where corresponds to beginning of
//               the segment and 1 corresponds to the end.
////////////////////////////////////////////////////////////////////

LColor ColorInterpolationSegment::
interpolateColor(const PN_stdfloat t) const {
  return _color_inter_func->interpolate((t-_t_begin)/_t_total);
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::ColorInterpolationManager
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationManager::
ColorInterpolationManager() :
  _default_color(LColor(1.0f,1.0f,1.0f,1.0f)),
  _id_generator(0) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::ColorInterpolationManager
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationManager::
ColorInterpolationManager(const LColor &c) :
  _default_color(c),
  _id_generator(0) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::ColorInterpolationManager
//      Access : public
// Description : copy constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationManager::
ColorInterpolationManager(const ColorInterpolationManager& copy) :
  _default_color(copy._default_color),
  _i_segs(copy._i_segs),
  _id_generator(copy._id_generator) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::~ColorInterpolationManager
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

ColorInterpolationManager::
~ColorInterpolationManager() {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::add_constant
//      Access : public
// Description : Adds a constant segment of the specified color to the
//               manager and returns the segment's id as known
//               by the manager.
////////////////////////////////////////////////////////////////////

int ColorInterpolationManager::
add_constant(const PN_stdfloat time_begin, const PN_stdfloat time_end, const LColor &color, const bool is_modulated) {
  PT(ColorInterpolationFunctionConstant) fPtr = new ColorInterpolationFunctionConstant(color);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,is_modulated,_id_generator);

  _i_segs.push_back(sPtr);

  return _id_generator++;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::add_linear
//      Access : public
// Description : Adds a linear segment between two colors to the manager 
//               and returns the segment's id as known by the manager.
////////////////////////////////////////////////////////////////////

int ColorInterpolationManager::
add_linear(const PN_stdfloat time_begin, const PN_stdfloat time_end, const LColor &color_a, const LColor &color_b, const bool is_modulated) {
  PT(ColorInterpolationFunctionLinear) fPtr = new ColorInterpolationFunctionLinear(color_a, color_b);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,is_modulated,_id_generator);

  _i_segs.push_back(sPtr);

  return _id_generator++;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::add_stepwave
//      Access : public
// Description : Adds a stepwave segment of two colors to the manager 
//               and returns the segment's id as known by the manager.
////////////////////////////////////////////////////////////////////

int ColorInterpolationManager::
add_stepwave(const PN_stdfloat time_begin, const PN_stdfloat time_end, const LColor &color_a, const LColor &color_b, const PN_stdfloat width_a, const PN_stdfloat width_b,const bool is_modulated) {
  PT(ColorInterpolationFunctionStepwave) fPtr = new ColorInterpolationFunctionStepwave(color_a, color_b, width_a, width_b);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,is_modulated,_id_generator);

  _i_segs.push_back(sPtr);

  return _id_generator++;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::add_sinusoid
//      Access : public
// Description : Adds a stepwave segment of two colors and a specified
//               period to the manager and returns the segment's 
//               id as known by the manager.
////////////////////////////////////////////////////////////////////

int ColorInterpolationManager::
add_sinusoid(const PN_stdfloat time_begin, const PN_stdfloat time_end, const LColor &color_a, const LColor &color_b, const PN_stdfloat period,const bool is_modulated) {
  PT(ColorInterpolationFunctionSinusoid) fPtr = new ColorInterpolationFunctionSinusoid(color_a, color_b, period);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,is_modulated,_id_generator);

  _i_segs.push_back(sPtr);

  return _id_generator++;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::clear_segment
//      Access : public
// Description : Removes the segment of 'id' from the manager.
////////////////////////////////////////////////////////////////////

void ColorInterpolationManager::
clear_segment(const int seg_id) {
  pvector<PT(ColorInterpolationSegment)>::iterator iter;

  for(iter = _i_segs.begin();iter != _i_segs.end();++iter) {
    if( seg_id == (*iter)->get_id() ) {
        _i_segs.erase(iter);
        return;
      }
  }
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::clear_to_initial
//      Access : public
// Description : Removes all segments from the manager.
////////////////////////////////////////////////////////////////////

void ColorInterpolationManager::
clear_to_initial() {
  _i_segs.clear();
  _id_generator = 0;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::
//      Access : public
// Description : For time 'interpolated_time', this returns the
//               additive composite color of all segments that influence 
//               that instant in the particle's lifetime. If no segments
//               cover that time, the manager's default color is returned.
////////////////////////////////////////////////////////////////////

LColor ColorInterpolationManager::
generateColor(const PN_stdfloat interpolated_time) {
  bool segment_found = false;
  LColor out(_default_color);
  ColorInterpolationSegment *cur_seg;
  pvector<PT(ColorInterpolationSegment)>::iterator iter;

  for (iter = _i_segs.begin();iter != _i_segs.end();++iter) {
    cur_seg = (*iter);
    if( cur_seg->is_enabled() && 
        interpolated_time >= cur_seg->get_time_begin() 
        && interpolated_time <= cur_seg->get_time_end() ) {
      segment_found = true;
      LColor cur_color = cur_seg->interpolateColor(interpolated_time);
      if( cur_seg->is_modulated() ) {
        out[0] *= cur_color[0];
        out[1] *= cur_color[1];
        out[2] *= cur_color[2];
        out[3] *= cur_color[3];
      }
      else {
        out[0] += cur_color[0];
        out[1] += cur_color[1];
        out[2] += cur_color[2];
        out[3] += cur_color[3];
      }
    }
  }
  
  if(segment_found) {
      out[0] = max((PN_stdfloat)0.0, min(out[0], (PN_stdfloat)1.0));
      out[1] = max((PN_stdfloat)0.0, min(out[1], (PN_stdfloat)1.0));
      out[2] = max((PN_stdfloat)0.0, min(out[2], (PN_stdfloat)1.0));
      out[3] = max((PN_stdfloat)0.0, min(out[3], (PN_stdfloat)1.0));
    return out;
  }
  
  return _default_color;
}

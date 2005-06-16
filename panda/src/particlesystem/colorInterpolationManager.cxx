// Filename: colorInterpolationManager.cxx
// Created by:  joswilso (02Jun05)
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
ColorInterpolationFunction(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunction::~ColorInterpolationFunction
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunction::
~ColorInterpolationFunction(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::ColorInterpolationFunctionConstant
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionConstant::
ColorInterpolationFunctionConstant(void) :
  _c_a(1.0f,1.0f,1.0f,1.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::ColorInterpolationFunctionConstant
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionConstant::
ColorInterpolationFunctionConstant(const Colorf color_a) :
  _c_a(color_a) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::interpolate
//      Access : protected
// Description : Returns the color associated with this instance.
////////////////////////////////////////////////////////////////////

Colorf ColorInterpolationFunctionConstant::
interpolate(const float t) const {
  return _c_a;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::ColorInterpolationFunctionLinear
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionLinear::
ColorInterpolationFunctionLinear(void) :
  _c_b(1.0f,1.0f,1.0f,1.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::ColorInterpolationFunctionLinear
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionLinear::
ColorInterpolationFunctionLinear(const Colorf color_a, 
                                 const Colorf color_b) :
  ColorInterpolationFunctionConstant(color_a),
  _c_b(color_b) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::interpolate
//      Access : protected
// Description : Returns the linear mixture of A and B according to 't'.
////////////////////////////////////////////////////////////////////

Colorf ColorInterpolationFunctionLinear::
interpolate(const float t) const {
  return (1.0f-t)*_c_a + t*_c_b;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::ColorInterpolationFunctionStepwave
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionStepwave::
ColorInterpolationFunctionStepwave(void) :
  _w_a(0.5f),
  _w_b(0.5f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::ColorInterpolationFunctionStepwave
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionStepwave::
ColorInterpolationFunctionStepwave(const Colorf color_a,
                                   const Colorf color_b, 
                                   const float width_a,
                                   const float width_b) :
  ColorInterpolationFunctionLinear(color_a,color_b),
  _w_a(width_a),
  _w_b(width_b) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::interpolate
//      Access : protected
// Description : Returns either A or B.
////////////////////////////////////////////////////////////////////

Colorf ColorInterpolationFunctionStepwave::
interpolate(const float t) const { 
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
ColorInterpolationFunctionSinusoid(void) :
  _period(1.0f) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionSinusoid::ColorInterpolationFunctionSinusoid
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationFunctionSinusoid::
ColorInterpolationFunctionSinusoid(const Colorf color_a, 
                                   const Colorf color_b, 
                                   const float period) :
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

Colorf ColorInterpolationFunctionSinusoid::
interpolate(const float t) const {
  float weight_a = (1.0f+cos(t*MathNumbers::pi_f*2.0f/_period))/2.0f;
  return (weight_a*_c_a)+((1.0f-weight_a)*_c_b);
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::ColorInterpolationSegment
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationSegment::
ColorInterpolationSegment(ColorInterpolationFunction* function,
                          const float &time_begin,
                          const float &time_end,
                          const int id) :
  _color_inter_func(function),
  _t_begin(time_begin),
  _t_end(time_end),
  _t_total(time_end-time_begin),
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
  _enabled(copy._enabled),
  _id(copy._id) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::~ColorInterpolationSegment
//      Access : public
// Description : destructor
////////////////////////////////////////////////////////////////////

ColorInterpolationSegment::
~ColorInterpolationSegment(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::interpolateColor
//      Access : public
// Description : Returns the interpolated color according to the
//               segment's function and start and end times.  't' is
//               a value in [0-1] where corresponds to beginning of
//               the segment and 1 corresponds to the end.
////////////////////////////////////////////////////////////////////

Colorf ColorInterpolationSegment::
interpolateColor(const float t) const {
  return _color_inter_func->interpolate((t-_t_begin)/_t_total);
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::ColorInterpolationManager
//      Access : public
// Description : default constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationManager::
ColorInterpolationManager(void) :
  _default_color(Colorf(1.0f,1.0f,1.0f,1.0f)),
  _id_generator(0) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::ColorInterpolationManager
//      Access : public
// Description : constructor
////////////////////////////////////////////////////////////////////

ColorInterpolationManager::
ColorInterpolationManager(const Colorf &c) :
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
~ColorInterpolationManager(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::add_constant
//      Access : public
// Description : Adds a constant segment of the specified color to the
//               manager and returns the segment's id as known
//               by the manager.
////////////////////////////////////////////////////////////////////

int ColorInterpolationManager::
add_constant(const float time_begin, const float time_end, const Colorf color) {
  PT(ColorInterpolationFunctionConstant) fPtr = new ColorInterpolationFunctionConstant(color);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,_id_generator);

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
add_linear(const float time_begin, const float time_end, const Colorf color_a, const Colorf color_b) {
  PT(ColorInterpolationFunctionLinear) fPtr = new ColorInterpolationFunctionLinear(color_a, color_b);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,_id_generator);

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
add_stepwave(const float time_begin, const float time_end, const Colorf color_a, const Colorf color_b, const float width_a, const float width_b) {
  PT(ColorInterpolationFunctionStepwave) fPtr = new ColorInterpolationFunctionStepwave(color_a, color_b, width_a, width_b);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,_id_generator);

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
add_sinusoid(const float time_begin, const float time_end, const Colorf color_a, const Colorf color_b, const float period) {
  PT(ColorInterpolationFunctionSinusoid) fPtr = new ColorInterpolationFunctionSinusoid(color_a, color_b, period);
  PT(ColorInterpolationSegment) sPtr = new ColorInterpolationSegment(fPtr,time_begin,time_end,_id_generator);

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
clear_to_initial(void) {
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

Colorf ColorInterpolationManager::
generateColor(const float interpolated_time) {
  bool segment_found = false;
  Colorf out(0.0f,0.0f,0.0f,0.0f);
  ColorInterpolationSegment *cur_seg;
  pvector<PT(ColorInterpolationSegment)>::iterator iter;

  for(iter = _i_segs.begin();iter != _i_segs.end();++iter) {
      cur_seg = (*iter);
      if( cur_seg->is_enabled() && 
          interpolated_time >= cur_seg->get_time_begin() 
          && interpolated_time <= cur_seg->get_time_end() ) {
          segment_found = true;
          out += cur_seg->interpolateColor(interpolated_time);
          out[0] = max(0,min(out[0],1.0f));
          out[1] = max(0,min(out[1],1.0f));
          out[2] = max(0,min(out[2],1.0f));
          out[3] = max(0,min(out[3],1.0f));
        }
    }
  
  if(segment_found) {
      return out;
  }

  return _default_color;
}

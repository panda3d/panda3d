// Filename: colorInterpolationManager.I
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

//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::get_color_a
//      Access : public
// Description : Returns the primary color of the function.
////////////////////////////////////////////////////////////////////

INLINE Colorf ColorInterpolationFunctionConstant::
get_color_a(void) const {
  return _c_a;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionConstant::set_color_a
//      Access : public
// Description : Sets the primary color of the function.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationFunctionConstant::
set_color_a(const Colorf c) {
  _c_a = c;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::get_color_b
//      Access : public
// Description : Returns the secondary color of the function.
////////////////////////////////////////////////////////////////////

INLINE Colorf ColorInterpolationFunctionLinear::
get_color_b(void) const {
  return _c_b;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionLinear::set_color_b
//      Access : public
// Description : Sets the secondary color of the function.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationFunctionLinear::
set_color_b(const Colorf c) {
  _c_b = c;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::get_width_a
//      Access : public
// Description : Returns the primary width of the function.
////////////////////////////////////////////////////////////////////

INLINE float ColorInterpolationFunctionStepwave::
get_width_a(void) const {
  return _w_a;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::get_width_b
//      Access : public
// Description : Returns the secondary width of the function.
////////////////////////////////////////////////////////////////////

INLINE float ColorInterpolationFunctionStepwave::
get_width_b(void) const {
  return _w_b;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::set_width_a
//      Access : public
// Description : Sets the primary width of the function.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationFunctionStepwave::
set_width_a(const float w) {
  _w_a = w;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionStepwave::set_width_b
//      Access : public
// Description : Sets the secondary width of the function.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationFunctionStepwave::
set_width_b(const float w) {
  _w_b = w;
}


////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionSinusoid::get_period
//      Access : public
// Description : Returns the time to transition from A to B then back
//               to A again.
////////////////////////////////////////////////////////////////////

INLINE float ColorInterpolationFunctionSinusoid::
get_period(void) const {
  return _period;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationFunctionSinusoid::set_period
//      Access : public
// Description : Sets the time to transition from A to B then back
//               to A again.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationFunctionSinusoid::
set_period(const float p) {
  _period = p;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::get_function
//      Access : public
// Description : Returns a reference to the function object 
//               corresponding to this segment.
////////////////////////////////////////////////////////////////////

INLINE TypedReferenceCount* ColorInterpolationSegment::
get_function(void) const {
  return _color_inter_func;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::get_time_begin
//      Access : public
// Description : Returns the point in the particle's lifetime at which
//               this segment begins its effect.  It is an interpolated
//               value in the range [0,1].
////////////////////////////////////////////////////////////////////

INLINE float ColorInterpolationSegment::
get_time_begin(void) const {
  return _t_begin;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::get_time_end
//      Access : public
// Description : Returns the point in the particle's lifetime at which
//               this segment's effect stops.  It is an interpolated
//               value in the range [0,1].
////////////////////////////////////////////////////////////////////

INLINE float ColorInterpolationSegment::
get_time_end(void) const {
  return _t_end;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::is_enabled()
//      Access : public
// Description : Returns whether the segments effects are being applied.
////////////////////////////////////////////////////////////////////

INLINE bool ColorInterpolationSegment::
is_enabled(void) const {
  return _enabled;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::get_id
//      Access : public
// Description : Returns the id assigned to this segment by the
//               manager that created it.
////////////////////////////////////////////////////////////////////

INLINE int ColorInterpolationSegment::
get_id(void) const {
  return _id;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::set_function
//      Access : public
// Description : Sets the function that the segment will use for 
//               its interpolation calculations.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationSegment::
set_function(ColorInterpolationFunction* function) {
  _color_inter_func = function;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::set_time_begin
//      Access : public
// Description : Sets the point in the particle's lifetime at which
//               this segment begins its effect.  It is an interpolated
//               value in the range [0,1].
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationSegment::
set_time_begin(const float time) {
  _t_begin = time;
  _t_total = _t_end-_t_begin;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::set_time_end
//      Access : public
// Description : Sets the point in the particle's lifetime at which
//               this segment's effect ends.  It is an interpolated
//               value in the range [0,1].
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationSegment::
set_time_end(const float time) {
  _t_end = time;
  _t_total = _t_end-_t_begin;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationSegment::set_enabled()
//      Access : public
// Description : Sets whether the segments effects should be applied.
////////////////////////////////////////////////////////////////////

INLINE void ColorInterpolationSegment::
set_enabled(const bool enabled) {
  _enabled = enabled;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::get_segment
//      Access : public
// Description : Returns the segment that corresponds to 'seg_id'.
////////////////////////////////////////////////////////////////////

INLINE ColorInterpolationSegment* ColorInterpolationManager::
get_segment(const int seg_id) {
  pvector<PT(ColorInterpolationSegment)>::iterator iter;

  for(iter = _i_segs.begin();iter != _i_segs.end();++iter)
    if( seg_id == (*iter)->get_id() )
      return (*iter);

  return NULL;
}

////////////////////////////////////////////////////////////////////
//    Function : ColorInterpolationManager::get_segment_id_list
//      Access : public
// Description : Returns a space delimited list of all of the ids
//               in the manager at the time.
////////////////////////////////////////////////////////////////////

INLINE string ColorInterpolationManager::
get_segment_id_list(void) {
  pvector<PT(ColorInterpolationSegment)>::iterator iter;
  ostringstream output;

  for(iter = _i_segs.begin();iter != _i_segs.end();++iter)
    output << (*iter)->get_id() << " ";

  string str = output.str();
  return str.substr(0, str.length()-1);
}

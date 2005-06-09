// Filename: colorInterpolationManager.h
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

#ifndef COLORINTERPOLATIONMANAGER_H
#define COLORINTERPOLATIONMANAGER_H

#include "luse.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationFunction
// Description : Abstract class from which all other functions 
//               should inherit. Defines the virtual interpolate()
//               function.
////////////////////////////////////////////////////////////////////

class ColorInterpolationFunction : public ReferenceCount {
PUBLISHED:
  virtual string get_type(void);
  
public:
  ColorInterpolationFunction(void);
  virtual ~ColorInterpolationFunction(void);

  virtual Colorf interpolate(const float t = 0) const = 0;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationFunctionConstant
// Description : Defines a constant color over the lifetime of
//               the segment.
////////////////////////////////////////////////////////////////////

class ColorInterpolationFunctionConstant : public ColorInterpolationFunction {
PUBLISHED:
  INLINE Colorf get_color_a(void) const;

  INLINE void set_color_a(const Colorf c);

public:
  ColorInterpolationFunctionConstant(void);
  ColorInterpolationFunctionConstant(const Colorf color_a);

protected:
  virtual Colorf interpolate(const float t = 0) const;
  virtual string get_type(void);

  Colorf _c_a;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationFunctionLinear
// Description : Defines a linear interpolation over the lifetime of
//               the segment.
////////////////////////////////////////////////////////////////////

class ColorInterpolationFunctionLinear : public ColorInterpolationFunctionConstant {
PUBLISHED:
  INLINE Colorf get_color_b(void) const;

  INLINE void set_color_b(const Colorf c);

public:
  ColorInterpolationFunctionLinear(void);
  ColorInterpolationFunctionLinear(const Colorf color_a, const Colorf color_b);

protected:
  Colorf interpolate(const float t = 0) const;
  virtual string get_type(void);

  Colorf _c_b;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationFunctionStepwave
// Description : Defines a discrete cyclical transition between two colors.
//               The widths describe a portion of the segment's lifetime
//               for which the corresponding color should be selected. If
//               their sum is less than 1, the function repeats until
//               the end of the segment.
////////////////////////////////////////////////////////////////////

class ColorInterpolationFunctionStepwave : public ColorInterpolationFunctionLinear {
PUBLISHED:
  INLINE float get_width_a(void) const;
  INLINE float get_width_b(void) const;

  INLINE void set_width_a(const float w);
  INLINE void set_width_b(const float w);

public:
  ColorInterpolationFunctionStepwave(void);
  ColorInterpolationFunctionStepwave(const Colorf color_a, const Colorf color_b, const float width_a, const float width_b);

protected:
  Colorf interpolate(const float t = 0) const;
  virtual string get_type(void);

  float _w_a;
  float _w_b;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationFunctionSinusoid
// Description : Defines a sinusoidal blending between two colors.
//               A period of "1" corresponds to a single transition 
//               from color_a to color_b and then back to color_a 
//               over the course of the segment's lifetime. A 
//               shorter period will result in a higher frequency
//               cycle.
////////////////////////////////////////////////////////////////////

class ColorInterpolationFunctionSinusoid : public ColorInterpolationFunctionLinear {
PUBLISHED:
  INLINE float get_period(void) const;

  INLINE void set_period(const float p);

public:
  ColorInterpolationFunctionSinusoid(void);
  ColorInterpolationFunctionSinusoid(const Colorf color_a, const Colorf color_b, const float period);

protected:
  Colorf interpolate(const float t = 0) const;
  virtual string get_type(void);

  float _period;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationSegment
// Description : A single unit of interpolation. The begin and end
//               times are interpolated over the lifetime of the 
//               particle, thus have the range of [0,1]. Each segment
//               also has a function associated with it.
////////////////////////////////////////////////////////////////////

class ColorInterpolationSegment : public ReferenceCount {
PUBLISHED:
  ColorInterpolationSegment(ColorInterpolationFunction* function, const float &time_begin, const float &time_end, const int id);
  ColorInterpolationSegment(const ColorInterpolationSegment &s);
  virtual ~ColorInterpolationSegment(void);

  INLINE ColorInterpolationFunction* get_function(void) const;
  INLINE float get_time_begin(void) const;
  INLINE float get_time_end(void) const;
  INLINE int get_id(void) const;

  INLINE void set_function(ColorInterpolationFunction* function);
  INLINE void set_time_begin(const float time);
  INLINE void set_time_end(const float time);

public:
  Colorf interpolateColor(const float t) const;
    
protected:
  PT(ColorInterpolationFunction) _color_inter_func;
  float _t_begin;
  float _t_end;
  float _t_total;
  const int _id;
};

////////////////////////////////////////////////////////////////////
//       Class : ColorInterpolationManager
// Description : High level class for color interpolation.  Segments
//               must be added to the manager in order to achieve
//               results using the "add_*****()" functions.  Access
//               to these segments is provided but not necessary
//               general use.
////////////////////////////////////////////////////////////////////

class ColorInterpolationManager : public ReferenceCount {
PUBLISHED:
ColorInterpolationManager(void);
  ColorInterpolationManager(const Colorf &c);
  ColorInterpolationManager(const ColorInterpolationManager& copy);    
  virtual ~ColorInterpolationManager(void);

  int add_constant(const float time_begin = 0.0f, const float time_end = 1.0f, const Colorf color = Colorf(1.0f,1.0f,1.0f,1.0f));
  int add_linear(const float time_begin = 0.0f, const float time_end = 1.0f, const Colorf color_a = Colorf(1.0f,0.0f,0.0f,1.0f), const Colorf color_b = Colorf(0.0f,1.0f,0.0f,1.0f));
  int add_stepwave(const float time_begin = 0.0f, const float time_end = 1.0f, const Colorf color_a = Colorf(1.0f,0.0f,0.0f,1.0f), const Colorf color_b = Colorf(0.0f,1.0f,0.0f,1.0f), const float width_a = 0.5f, const float width_b = 0.5f);
  int add_sinusoid(const float time_begin = 0.0f, const float time_end = 1.0f, const Colorf color_a = Colorf(1.0f,0.0f,0.0f,1.0f), const Colorf color_b = Colorf(0.0f,1.0f,0.0f,1.0f), const float period = 1.0f);

  static ColorInterpolationFunctionConstant* downcast_function_to_constant(ColorInterpolationFunction* f);
  static ColorInterpolationFunctionLinear*   downcast_function_to_linear(ColorInterpolationFunction* f);
  static ColorInterpolationFunctionStepwave* downcast_function_to_stepwave(ColorInterpolationFunction* f);
  static ColorInterpolationFunctionSinusoid* downcast_function_to_sinusoid(ColorInterpolationFunction* f);

  INLINE ColorInterpolationSegment* get_segment(const int seg_id);
  INLINE string get_segment_id_list(void);
  void clear_segment(const int seg_id);
  void clear_to_initial(void);

public:
  Colorf generateColor(const float interpolated_time);

private:
  Colorf _default_color;
  pvector<PT(ColorInterpolationSegment)> _i_segs;
  int _id_generator;
};

#include "colorInterpolationManager.I"

#endif //COLORINTERPOLATIONMANAGER_H

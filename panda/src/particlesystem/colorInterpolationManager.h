/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorInterpolationManager.h
 * @author joswilso
 * @date 2005-06-02
 */

#ifndef COLORINTERPOLATIONMANAGER_H
#define COLORINTERPOLATIONMANAGER_H

#include "luse.h"
#include "pvector.h"
#include "typedObject.h"
#include "typedReferenceCount.h"

/**
 * Abstract class from which all other functions should inherit.  Defines the
 * virtual interpolate() function.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationFunction : public TypedReferenceCount {
PUBLISHED:
// virtual string get_type();

public:
  ColorInterpolationFunction();
  virtual ~ColorInterpolationFunction();

  virtual LColor interpolate(const PN_stdfloat t = 0) const = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ColorInterpolationFunction",
                  TypedReferenceCount::get_class_type());
  }

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
private:
  static TypeHandle _type_handle;
};

/**
 * Defines a constant color over the lifetime of the segment.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationFunctionConstant : public ColorInterpolationFunction {
PUBLISHED:
  INLINE LColor get_color_a() const;

  INLINE void set_color_a(const LColor &c);

public:
  ColorInterpolationFunctionConstant();
  ColorInterpolationFunctionConstant(const LColor &color_a);

protected:
  virtual LColor interpolate(const PN_stdfloat t = 0) const;
  // virtual string get_type();

  LColor _c_a;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    ColorInterpolationFunction::init_type();
    register_type(_type_handle, "ColorInterpolationFunctionConstant",
                  ColorInterpolationFunction::get_class_type());
  }

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
private:
  static TypeHandle _type_handle;
};

/**
 * Defines a linear interpolation over the lifetime of the segment.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationFunctionLinear : public ColorInterpolationFunctionConstant {
PUBLISHED:
  INLINE LColor get_color_b() const;

  INLINE void set_color_b(const LColor &c);

public:
  ColorInterpolationFunctionLinear();
  ColorInterpolationFunctionLinear(const LColor &color_a, const LColor &color_b);

protected:
  LColor interpolate(const PN_stdfloat t = 0) const;
  // virtual string get_type();

  LColor _c_b;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    ColorInterpolationFunctionConstant::init_type();
    register_type(_type_handle, "ColorInterpolationFunctionLinear",
                  ColorInterpolationFunctionConstant::get_class_type());
  }

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
private:
  static TypeHandle _type_handle;
};

/**
 * Defines a discrete cyclical transition between two colors.  The widths
 * describe a portion of the segment's lifetime for which the corresponding
 * color should be selected.  If their sum is less than 1, the function
 * repeats until the end of the segment.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationFunctionStepwave : public ColorInterpolationFunctionLinear {
PUBLISHED:
  INLINE PN_stdfloat get_width_a() const;
  INLINE PN_stdfloat get_width_b() const;

  INLINE void set_width_a(const PN_stdfloat w);
  INLINE void set_width_b(const PN_stdfloat w);

public:
  ColorInterpolationFunctionStepwave();
  ColorInterpolationFunctionStepwave(const LColor &color_a, const LColor &color_b, const PN_stdfloat width_a, const PN_stdfloat width_b);

protected:
  LColor interpolate(const PN_stdfloat t = 0) const;
  // virtual string get_type();

  PN_stdfloat _w_a;
  PN_stdfloat _w_b;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    ColorInterpolationFunctionLinear::init_type();
    register_type(_type_handle, "ColorInterpolationFunctionStepwave",
                  ColorInterpolationFunctionLinear::get_class_type());
  }

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
private:
  static TypeHandle _type_handle;
};

/**
 * Defines a sinusoidal blending between two colors.  A period of "1"
 * corresponds to a single transition from color_a to color_b and then back to
 * color_a over the course of the segment's lifetime.  A shorter period will
 * result in a higher frequency cycle.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationFunctionSinusoid : public ColorInterpolationFunctionLinear {
PUBLISHED:
  INLINE PN_stdfloat get_period() const;

  INLINE void set_period(const PN_stdfloat p);

public:
  ColorInterpolationFunctionSinusoid();
  ColorInterpolationFunctionSinusoid(const LColor &color_a, const LColor &color_b, const PN_stdfloat period);

protected:
  LColor interpolate(const PN_stdfloat t = 0) const;
  // virtual string get_type();

  PN_stdfloat _period;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

  static void init_type() {
    ColorInterpolationFunctionLinear::init_type();
    register_type(_type_handle, "ColorInterpolationFunctionSinusoid",
                  ColorInterpolationFunctionLinear::get_class_type());
  }

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
private:
  static TypeHandle _type_handle;
};

/**
 * A single unit of interpolation.  The begin and end times are interpolated
 * over the lifetime of the particle, thus have the range of [0,1]. Each
 * segment also has a function associated with it.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationSegment : public ReferenceCount {
public:
  ColorInterpolationSegment(ColorInterpolationFunction* function, const PN_stdfloat &time_begin, const PN_stdfloat &time_end, const bool is_modulated, const int id);

PUBLISHED:
  ColorInterpolationSegment(const ColorInterpolationSegment &s);

  virtual ~ColorInterpolationSegment();

  // INLINE ColorInterpolationFunction* get_function() const;
  INLINE TypedReferenceCount* get_function() const;
  INLINE PN_stdfloat get_time_begin() const;
  INLINE PN_stdfloat get_time_end() const;
  INLINE bool is_modulated() const;
  INLINE int get_id() const;
  INLINE bool is_enabled() const;

  INLINE void set_function(ColorInterpolationFunction* function);
  INLINE void set_time_begin(const PN_stdfloat time);
  INLINE void set_time_end(const PN_stdfloat time);
  INLINE void set_is_modulated(const bool flag);
  INLINE void set_enabled(const bool enabled);

public:
  LColor interpolateColor(const PN_stdfloat t) const;

protected:
  PT(ColorInterpolationFunction) _color_inter_func;
  PN_stdfloat _t_begin;
  PN_stdfloat _t_end;
  PN_stdfloat _t_total;
  bool _is_modulated;
  bool _enabled;
  const int _id;
};

/**
 * High level class for color interpolation.  Segments must be added to the
 * manager in order to achieve results using the "add_*****()" functions.
 * Access to these segments is provided but not necessary general use.
 */

class EXPCL_PANDA_PARTICLESYSTEM ColorInterpolationManager : public ReferenceCount {
PUBLISHED:
ColorInterpolationManager();
  ColorInterpolationManager(const LColor &c);
  ColorInterpolationManager(const ColorInterpolationManager& copy);
  virtual ~ColorInterpolationManager();

  int add_constant(const PN_stdfloat time_begin = 0.0f, const PN_stdfloat time_end = 1.0f, const LColor &color = LColor(1.0f,1.0f,1.0f,1.0f), const bool is_modulated = true);
  int add_linear(const PN_stdfloat time_begin = 0.0f, const PN_stdfloat time_end = 1.0f, const LColor &color_a = LColor(1.0f,0.0f,0.0f,1.0f), const LColor &color_b = LColor(0.0f,1.0f,0.0f,1.0f), const bool is_modulated = true);
  int add_stepwave(const PN_stdfloat time_begin = 0.0f, const PN_stdfloat time_end = 1.0f, const LColor &color_a = LColor(1.0f,0.0f,0.0f,1.0f), const LColor &color_b = LColor(0.0f,1.0f,0.0f,1.0f), const PN_stdfloat width_a = 0.5f, const PN_stdfloat width_b = 0.5f, const bool is_modulated = true);
  int add_sinusoid(const PN_stdfloat time_begin = 0.0f, const PN_stdfloat time_end = 1.0f, const LColor &color_a = LColor(1.0f,0.0f,0.0f,1.0f), const LColor &color_b = LColor(0.0f,1.0f,0.0f,1.0f), const PN_stdfloat period = 1.0f, const bool is_modulated = true);

  INLINE void set_default_color(const LColor &c);
  INLINE ColorInterpolationSegment* get_segment(const int seg_id);
  INLINE std::string get_segment_id_list();
  void clear_segment(const int seg_id);
  void clear_to_initial();

public:
  LColor generateColor(const PN_stdfloat interpolated_time);

private:
  LColor _default_color;
  pvector<PT(ColorInterpolationSegment)> _i_segs;
  int _id_generator;
};

#include "colorInterpolationManager.I"

#endif //COLORINTERPOLATIONMANAGER_H

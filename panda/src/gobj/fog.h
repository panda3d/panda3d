// Filename: fog.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef FOG_H
#define FOG_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsStateGuardianBase.h>
#include <typedReferenceCount.h>
#include <luse.h>
#include <pointerToArray.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Fog
// Description : Specifies atmospheric fog parameters
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Fog : public TypedReferenceCount {
PUBLISHED:
  enum Mode {
    M_linear,                   // f = (end - z) / (end - start)
    M_exponential,              // f = e^(-density * z)
    M_super_exponential,        // f = e^(-density * z)^2
    M_spline,                   // Not implemented yet
  };

  Fog(Mode mode = M_linear, int hardware_bits = 8);
  ~Fog();

  INLINE Mode get_mode(void) const;
  INLINE void set_mode(Mode mode);

  INLINE Colorf get_color(void) const;
  INLINE void set_color(const Colorf &color);

  INLINE void get_range(float &onset, float &opaque) const;
  INLINE void set_range(float onset, float opaque);
 
  INLINE void get_offsets(float &onset, float &opaque) const;
  INLINE void set_offsets(float onset, float opaque);

  INLINE float get_start(void) const;
  INLINE float get_end(void) const;
  INLINE float get_density(void) const;

  void output(ostream &out) const;

public:
  INLINE void apply(GraphicsStateGuardianBase *gsg);

protected:
  void compute_density(void);

protected:
  Mode			_mode;
  int			_hardware_bits;
  Colorf		_color;
  float			_onset;
  float			_opaque;
  float			_onset_offset;
  float			_opaque_offset;
  float 		_density;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Fog",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

ostream &operator << (ostream &out, Fog::Mode mode);

INLINE ostream &operator << (ostream &out, const Fog &fog) {
  fog.output(out);
  return out;
}

#include "fog.I"

#endif

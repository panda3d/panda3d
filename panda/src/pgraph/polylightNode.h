// Filename: PolylightNode.h
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

#ifndef POLYLIGHTNODE_H
#define POLYLIGHTNODE_H

#include "pandabase.h"

#include "luse.h"
#include "nodePath.h"
#include "pmap.h"
#include "notify.h"
#include "pandaNode.h"
#include "colorAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : PolylightNode
// Description : A PolylightNode
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolylightNode : public PandaNode{
//private:


PUBLISHED:
  /*
  // This was the old constructor... interrogate would generate a 
  // separate wrapper for each parameter... so its better to 
  // have a simpler constructor and require the programmer
  // to use set_* methods.
  PolylightNode(const string &name, float x = 0.0, float y = 0.0, float z = 0.0,
  float r = 1.0, float g = 1.0, float b = 1.0,
  float radius=50.0, string attenuation_type= "linear",
  bool flickering =false, string flicker_type="random");
  */

  enum Flicker_Type {
    FRANDOM,
    FSIN,
    FCUSTOM,
  };

  enum Attenuation_Type {
    ALINEAR,
    AQUADRATIC,
  };

  PolylightNode(const string &name);
  INLINE void enable();
  INLINE void disable();
  INLINE void set_pos(LVecBase3f position);
  INLINE void set_pos(float x,float y, float z);
  INLINE LVecBase3f get_pos() const;
  INLINE void set_color(Colorf color);
  INLINE void set_color(float r, float g, float b);
  INLINE Colorf get_color() const;
  INLINE Colorf get_color_scenegraph() const;
  INLINE void set_radius(float r);
  INLINE float get_radius() const;
  INLINE bool set_attenuation(Attenuation_Type type);
  INLINE Attenuation_Type get_attenuation() const;
  INLINE void set_a0(float a0);
  INLINE void set_a1(float a1);
  INLINE void set_a2(float a2);
  INLINE float get_a0() const;
  INLINE float get_a1() const;
  INLINE float get_a2() const;
  INLINE void flicker_on();
  INLINE void flicker_off();
  INLINE bool is_flickering() const;
  INLINE bool set_flicker_type(Flicker_Type type);
  INLINE Flicker_Type get_flicker_type() const;
  INLINE void set_offset(float offset);
  INLINE float get_offset() const;
  INLINE void set_scale(float scale);
  INLINE float get_scale() const;
  INLINE void set_step_size(float step) ;
  INLINE float get_step_size() const;
  INLINE void set_freq(float f);
  INLINE float get_freq() const;

  // Comparison methods
  INLINE bool operator == (const PolylightNode &other) const;
  INLINE bool operator != (const PolylightNode &other) const;
  INLINE bool operator < (const PolylightNode &other) const;
  int compare_to(const PolylightNode &other) const;

  INLINE bool is_enabled() const;

public:
  Colorf flicker() const;

  
private:
  bool _enabled;
  LVecBase3f _position;
  Colorf _color;
  float _radius;
  Attenuation_Type _attenuation_type;
  float _a0;
  float _a1;
  float _a2;
  bool _flickering;
  Flicker_Type _flicker_type;
  float _offset;
  float _scale;
  float _step_size;
  float _sin_freq;
  //float _speed;
  //float fixed_points
  

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void output(ostream &out) const;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);



public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PolylightNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "polylightNode.I"

#endif



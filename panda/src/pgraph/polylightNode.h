/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file polylightNode.h
 * @author sshodhan
 * @date 2004-06-02
 */

#ifndef POLYLIGHTNODE_H
#define POLYLIGHTNODE_H

#include "pandabase.h"

#include "luse.h"
#include "nodePath.h"
#include "pmap.h"
#include "pnotify.h"
#include "pandaNode.h"
#include "colorAttrib.h"

/**
 * A PolylightNode
 */
class EXPCL_PANDA_PGRAPH PolylightNode : public PandaNode{
// private:


PUBLISHED:
  /*
  // This was the old constructor... interrogate would generate a separate
  // wrapper for each parameter... so its better to have a simpler constructor
  // and require the programmer to use set_* methods.
  PolylightNode(const string &name, PN_stdfloat x = 0.0, PN_stdfloat y = 0.0, PN_stdfloat z = 0.0,
  PN_stdfloat r = 1.0, PN_stdfloat g = 1.0, PN_stdfloat b = 1.0,
  PN_stdfloat radius=50.0, string attenuation_type= "linear",
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

  explicit PolylightNode(const std::string &name);
  INLINE void enable();
  INLINE void disable();
  INLINE void set_pos(const LPoint3 &position);
  INLINE void set_pos(PN_stdfloat x,PN_stdfloat y, PN_stdfloat z);
  INLINE LPoint3 get_pos() const;
  INLINE void set_color(const LColor &color);
  INLINE void set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b);
  INLINE LColor get_color() const;
  INLINE LColor get_color_scenegraph() const;
  INLINE void set_radius(PN_stdfloat r);
  INLINE PN_stdfloat get_radius() const;
  INLINE bool set_attenuation(Attenuation_Type type);
  INLINE Attenuation_Type get_attenuation() const;
  INLINE void set_a0(PN_stdfloat a0);
  INLINE void set_a1(PN_stdfloat a1);
  INLINE void set_a2(PN_stdfloat a2);
  INLINE PN_stdfloat get_a0() const;
  INLINE PN_stdfloat get_a1() const;
  INLINE PN_stdfloat get_a2() const;
  INLINE void flicker_on();
  INLINE void flicker_off();
  INLINE bool is_flickering() const;
  INLINE bool set_flicker_type(Flicker_Type type);
  INLINE Flicker_Type get_flicker_type() const;
  INLINE void set_offset(PN_stdfloat offset);
  INLINE PN_stdfloat get_offset() const;
  INLINE void set_scale(PN_stdfloat scale);
  INLINE PN_stdfloat get_scale() const;
  INLINE void set_step_size(PN_stdfloat step) ;
  INLINE PN_stdfloat get_step_size() const;
  INLINE void set_freq(PN_stdfloat f);
  INLINE PN_stdfloat get_freq() const;

  // Comparison methods
  INLINE bool operator == (const PolylightNode &other) const;
  INLINE bool operator != (const PolylightNode &other) const;
  INLINE bool operator < (const PolylightNode &other) const;
  int compare_to(const PolylightNode &other) const;

  INLINE bool is_enabled() const;

public:
  LColor flicker() const;
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);

private:
  bool _enabled;
  LPoint3 _position;
  LColor _color;
  PN_stdfloat _radius;
  Attenuation_Type _attenuation_type;
  PN_stdfloat _a0;
  PN_stdfloat _a1;
  PN_stdfloat _a2;
  bool _flickering;
  Flicker_Type _flicker_type;
  PN_stdfloat _offset;
  PN_stdfloat _scale;
  PN_stdfloat _step_size;
  PN_stdfloat _sin_freq;
  // PN_stdfloat _speed; PN_stdfloat fixed_points


public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void output(std::ostream &out) const;

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

// Filename: linearNoiseForce.h
// Created by:  charles (13Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef LINEARNOISEFORCE_H
#define LINEARNOISEFORCE_H

#include "pandabase.h"
#include "linearRandomForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearNoiseForce
// Description : Repeating noise force vector.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearNoiseForce : public LinearRandomForce {
private:
  static unsigned char _prn_table[256];
  static LVector3f _gradient_table[256];
  static bool _initialized;

  INLINE float cubic_step(const float x) const;
  INLINE LVector3f vlerp(const float t, const LVector3f& v0, const LVector3f& v1) const;

  INLINE unsigned char get_prn_entry(const LPoint3f& point) const;
  INLINE unsigned char get_prn_entry(const float x, const float y, const float z) const;

  INLINE LVector3f& get_lattice_entry(const LPoint3f& point);
  INLINE LVector3f& get_lattice_entry(const float x, const float y, const float z);

  INLINE unsigned char prn_lookup(int index) const;

  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

public:
  static int _random_seed;
  static void init_noise_tables(void);

PUBLISHED:
  LinearNoiseForce(float a = 1.0f, bool m = false);
  LinearNoiseForce(const LinearNoiseForce &copy);
  virtual ~LinearNoiseForce(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearRandomForce::init_type();
    register_type(_type_handle, "LinearNoiseForce",
                  LinearRandomForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearNoiseForce.I"

#endif // LINEARNOISEFORCE_H

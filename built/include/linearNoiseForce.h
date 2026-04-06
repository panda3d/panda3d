/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linearNoiseForce.h
 * @author charles
 * @date 2000-06-13
 */

#ifndef LINEARNOISEFORCE_H
#define LINEARNOISEFORCE_H

#include "pandabase.h"
#include "linearRandomForce.h"
#include "configVariableInt.h"

/**
 * Repeating noise force vector.
 */
class EXPCL_PANDA_PHYSICS LinearNoiseForce : public LinearRandomForce {
PUBLISHED:
  explicit LinearNoiseForce(PN_stdfloat a = 1.0f, bool m = false);
  LinearNoiseForce(const LinearNoiseForce &copy);
  virtual ~LinearNoiseForce();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent=0) const;

public:
  static ConfigVariableInt _random_seed;
  static void init_noise_tables();

private:
  static unsigned char _prn_table[256];
  static LVector3 _gradient_table[256];
  static bool _initialized;

  INLINE PN_stdfloat cubic_step(const PN_stdfloat x) const;
  INLINE LVector3 vlerp(const PN_stdfloat t, const LVector3& v0, const LVector3& v1) const;

  INLINE unsigned char get_prn_entry(const LPoint3& point) const;
  INLINE unsigned char get_prn_entry(const PN_stdfloat x, const PN_stdfloat y, const PN_stdfloat z) const;

  INLINE LVector3& get_lattice_entry(const LPoint3& point);
  INLINE LVector3& get_lattice_entry(const PN_stdfloat x, const PN_stdfloat y, const PN_stdfloat z);

  INLINE unsigned char prn_lookup(int index) const;

  virtual LVector3 get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LinearRandomForce::init_type();
    register_type(_type_handle, "LinearNoiseForce",
                  LinearRandomForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearNoiseForce.I"

#endif // LINEARNOISEFORCE_H

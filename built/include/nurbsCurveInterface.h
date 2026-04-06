/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurveInterface.h
 * @author drose
 * @date 2001-03-02
 */

#ifndef NURBSCURVEINTERFACE_H
#define NURBSCURVEINTERFACE_H

#include "pandabase.h"

#include "luse.h"
#include "filename.h"

class ParametricCurve;

/**
 * This abstract class defines the interface only for a Nurbs-style curve,
 * with knots and coordinates in homogeneous space.
 *
 * The NurbsCurve class inherits both from this and from ParametricCurve.
 */
class EXPCL_PANDA_PARAMETRICS NurbsCurveInterface {
PUBLISHED:
  virtual ~NurbsCurveInterface();
  virtual void set_order(int order)=0;
  virtual int get_order() const=0;

  virtual int get_num_cvs() const=0;
  virtual int get_num_knots() const=0;

  virtual bool insert_cv(PN_stdfloat t)=0;

  INLINE int append_cv(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE int append_cv(const LVecBase3 &v);
  INLINE int append_cv(const LVecBase4 &v);

  virtual bool remove_cv(int n)=0;
  virtual void remove_all_cvs()=0;

  INLINE bool set_cv_point(int n, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE bool set_cv_point(int n, const LVecBase3 &v);
  INLINE LVecBase3 get_cv_point(int n) const;

  bool set_cv_weight(int n, PN_stdfloat w);
  INLINE PN_stdfloat get_cv_weight(int n) const;

  virtual bool set_cv(int n, const LVecBase4 &v)=0;
  virtual LVecBase4 get_cv(int n) const=0;

  virtual bool set_knot(int n, PN_stdfloat t)=0;
  virtual PN_stdfloat get_knot(int n) const=0;

  MAKE_SEQ(get_cvs, get_num_cvs, get_cv);
  MAKE_SEQ(get_knots, get_num_knots, get_knot);

  void write_cv(std::ostream &out, int n) const;

protected:
  virtual int append_cv_impl(const LVecBase4 &v)=0;

  void write(std::ostream &out, int indent_level) const;
  bool format_egg(std::ostream &out, const std::string &name,
                  const std::string &curve_type, int indent_level) const;

  bool convert_to_nurbs(ParametricCurve *nc) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "NurbsCurveInterface");
  }

private:
  static TypeHandle _type_handle;
};

#include "nurbsCurveInterface.I"

#endif

// Filename: nurbsCurveInterface.h
// Created by:  drose (02Mar01)
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

#ifndef NURBSCURVEINTERFACE_H
#define NURBSCURVEINTERFACE_H

#include "pandabase.h"

#include "luse.h"
#include "filename.h"

class ParametricCurve;

////////////////////////////////////////////////////////////////////
//       Class : NurbsCurveInterface
// Description : This abstract class defines the interface only for a
//               Nurbs-style curve, with knots and coordinates in
//               homogeneous space.
//
//               The NurbsCurve class inherits both from this and from
//               ParametricCurve.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NurbsCurveInterface {
PUBLISHED:
  virtual void set_order(int order)=0;
  virtual int get_order() const=0;

  virtual int get_num_cvs() const=0;
  virtual int get_num_knots() const=0;

  virtual bool insert_cv(float t)=0;

  INLINE int append_cv(float x, float y, float z);
  INLINE int append_cv(const LVecBase3f &v);
  INLINE int append_cv(const LVecBase4f &v);

  virtual bool remove_cv(int n)=0;
  virtual void remove_all_cvs()=0;

  INLINE bool set_cv_point(int n, float x, float y, float z);
  INLINE bool set_cv_point(int n, const LVecBase3f &v);
  INLINE LVecBase3f get_cv_point(int n) const;

  bool set_cv_weight(int n, float w);
  INLINE float get_cv_weight(int n) const;

  virtual bool set_cv(int n, const LVecBase4f &v)=0;
  virtual LVecBase4f get_cv(int n) const=0;

  virtual bool set_knot(int n, float t)=0;
  virtual float get_knot(int n) const=0;

  void write_cv(ostream &out, int n) const;


protected:
  virtual int append_cv_impl(const LVecBase4f &v)=0;

  void write(ostream &out, int indent_level) const;
  bool format_egg(ostream &out, const string &name,
                  const string &curve_type, int indent_level) const;

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

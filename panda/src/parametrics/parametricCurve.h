/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parametricCurve.h
 * @author drose
 * @date 2001-03-04
 */

#ifndef PARAMETRICCURVE_H
#define PARAMETRICCURVE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "luse.h"

#include "typedef.h"
#include "plist.h"
#include "pvector.h"


// Parametric curve semantic types.  A parametric curve may have one of these
// types specified.  These serve as hints to the egg reader and writer code
// about the intention of this curve, and have no other effect on the curve.

BEGIN_PUBLISH //[
#define PCT_NONE        0
// Unspecified type.

#define PCT_XYZ         1
// A three-dimensional curve in space.

#define PCT_HPR         2
// The curve represents Euler rotation angles.

#define PCT_T           3
// A one-dimensional timewarp curve.
END_PUBLISH //]

class ParametricCurveDrawer;
class HermiteCurveCV;
class HermiteCurve;
class NurbsCurve;
class NurbsCurveInterface;


/**
 * A virtual base class for parametric curves.  This encapsulates all curves
 * in 3-d space defined for a single parameter t in the range [0,get_max_t()].
 */
class EXPCL_PANDA_PARAMETRICS ParametricCurve : public PandaNode {
PUBLISHED:
  ParametricCurve();
  virtual ~ParametricCurve();

public:
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;

PUBLISHED:
  virtual bool is_valid() const;

  virtual PN_stdfloat get_max_t() const;

  void set_curve_type(int type);
  int get_curve_type() const;

  void set_num_dimensions(int num);
  int get_num_dimensions() const;

  PN_stdfloat calc_length() const;
  PN_stdfloat calc_length(PN_stdfloat from, PN_stdfloat to) const;
  PN_stdfloat find_length(PN_stdfloat start_t, PN_stdfloat length_offset) const;

  virtual bool get_point(PN_stdfloat t, LVecBase3 &point) const=0;
  virtual bool get_tangent(PN_stdfloat t, LVecBase3 &tangent) const=0;
  virtual bool get_pt(PN_stdfloat t, LVecBase3 &point, LVecBase3 &tangent) const=0;
  virtual bool get_2ndtangent(PN_stdfloat t, LVecBase3 &tangent2) const=0;

  virtual bool adjust_point(PN_stdfloat t, PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz);
  virtual bool adjust_tangent(PN_stdfloat t, PN_stdfloat tx, PN_stdfloat ty, PN_stdfloat tz);
  virtual bool adjust_pt(PN_stdfloat t,
                         PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz,
                         PN_stdfloat tx, PN_stdfloat ty, PN_stdfloat tz);

  virtual bool recompute();

  virtual bool stitch(const ParametricCurve *a, const ParametricCurve *b);

  bool write_egg(Filename filename, CoordinateSystem cs = CS_default);
  bool write_egg(std::ostream &out, const Filename &filename, CoordinateSystem cs);

public:
  struct BezierSeg {
  public:
    LVecBase3 _v[4];
    PN_stdfloat _t;
  };
  typedef pvector<BezierSeg> BezierSegs;

  virtual bool get_bezier_segs(BezierSegs &) const;
  virtual bool get_bezier_seg(BezierSeg &) const;
  virtual NurbsCurveInterface *get_nurbs_interface();

  virtual bool convert_to_hermite(HermiteCurve *hc) const;
  virtual bool convert_to_nurbs(ParametricCurve *nc) const;

  void register_drawer(ParametricCurveDrawer *drawer);
  void unregister_drawer(ParametricCurveDrawer *drawer);

protected:
  void invalidate(PN_stdfloat t1, PN_stdfloat t2);
  void invalidate_all();

  virtual bool format_egg(std::ostream &out, const std::string &name,
                          const std::string &curve_type, int indent_level) const;

private:
  PN_stdfloat r_calc_length(PN_stdfloat t1, PN_stdfloat t2,
                      const LPoint3 &p1, const LPoint3 &p2,
                      PN_stdfloat seglength) const;
  bool r_find_length(PN_stdfloat target_length, PN_stdfloat &found_t,
                     PN_stdfloat t1, PN_stdfloat t2,
                     const LPoint3 &p1, const LPoint3 &p2,
                     PN_stdfloat &seglength) const;
  bool r_find_t(PN_stdfloat target_length, PN_stdfloat &found_t,
                PN_stdfloat t1, PN_stdfloat t2,
                const LPoint3 &p1, const LPoint3 &p2) const;
  bool find_t_linear(PN_stdfloat target_length, PN_stdfloat &found_t,
                     PN_stdfloat t1, PN_stdfloat t2,
                     const LPoint3 &p1, const LPoint3 &p2) const;

protected:
  int _curve_type;
  int _num_dimensions;

private:
  typedef plist<ParametricCurveDrawer *> DrawerList;
  DrawerList _drawers;

// TypedWritable stuff
protected:
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ParametricCurve",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif

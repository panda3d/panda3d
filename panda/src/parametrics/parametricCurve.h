// Filename: parametricCurve.h
// Created by:  drose (04Mar01)
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

#ifndef PARAMETRICCURVE_H
#define PARAMETRICCURVE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "luse.h"

#include "typedef.h"
#include "plist.h"
#include "pvector.h"


// Parametric curve semantic types.  A parametric curve may have one
// of these types specified.  These serve as hints to the egg reader
// and writer code about the intention of this curve, and have no
// other effect on the curve.

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
class ClassicNurbsCurve;
class NurbsCurveInterface;


////////////////////////////////////////////////////////////////////
//       Class : ParametricCurve
// Description : A virtual base class for parametric curves.
//               This encapsulates all curves in 3-d space defined
//               for a single parameter t in the range [0,get_max_t()].
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ParametricCurve : public PandaNode {
PUBLISHED:
  ParametricCurve();
  virtual ~ParametricCurve();

public:
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;

PUBLISHED:
  virtual bool is_valid() const;

  virtual float get_max_t() const;

  void set_curve_type(int type);
  int get_curve_type() const;

  void set_num_dimensions(int num);
  int get_num_dimensions() const;

  float calc_length() const;
  float calc_length(float from, float to) const;
  float find_length(float start_t, float length_offset) const;

  virtual bool get_point(float t, LVecBase3f &point) const=0;
  virtual bool get_tangent(float t, LVecBase3f &tangent) const=0;
  virtual bool get_pt(float t, LVecBase3f &point, LVecBase3f &tangent) const=0;
  virtual bool get_2ndtangent(float t, LVecBase3f &tangent2) const=0;

  virtual bool adjust_point(float t, float px, float py, float pz);
  virtual bool adjust_tangent(float t, float tx, float ty, float tz);
  virtual bool adjust_pt(float t,
                         float px, float py, float pz,
                         float tx, float ty, float tz);

  virtual bool recompute();

  virtual bool stitch(const ParametricCurve *a, const ParametricCurve *b);

  bool write_egg(Filename filename, CoordinateSystem cs = CS_default);
  bool write_egg(ostream &out, const Filename &filename, CoordinateSystem cs);

public:
  struct BezierSeg {
  public:
    LVecBase3f _v[4];
    float _t;
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
  void invalidate(float t1, float t2);
  void invalidate_all();

  virtual bool format_egg(ostream &out, const string &name,
                          const string &curve_type, int indent_level) const;

private:
  float r_calc_length(float t1, float t2,
                      const LPoint3f &p1, const LPoint3f &p2,
                      float seglength) const;
  bool r_find_length(float target_length, float &found_t,
                     float t1, float t2,
                     const LPoint3f &p1, const LPoint3f &p2,
                     float &seglength) const;
  bool r_find_t(float target_length, float &found_t,
                float t1, float t2,
                const LPoint3f &p1, const LPoint3f &p2) const;
  bool find_t_linear(float target_length, float &found_t,
		     float t1, float t2,
		     const LPoint3f &p1, const LPoint3f &p2) const;

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

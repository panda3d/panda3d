// Filename: nurbsCurve.C
// Created by:  drose (27Feb98)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#include "nurbsCurve.h"
#include "config_parametrics.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////

TypeHandle NurbsCurve::_type_handle;

static const LVecBase3f zero = LVecBase3f(0.0, 0.0, 0.0);
// This is returned occasionally from some of the functions, and is
// used from time to time as an initializer.


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::Constructor
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
NurbsCurve::
NurbsCurve() {
  _order = 4;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::Constructor
//       Access: Public, Scheme
//  Description: Constructs a NURBS curve equivalent to the indicated
//               (possibly non-NURBS) curve.
////////////////////////////////////////////////////////////////////
NurbsCurve::
NurbsCurve(const ParametricCurve &pc) {
  _order = 4;
  
  if (!pc.convert_to_nurbs(*this)) {
    parametrics_cat->warning() 
      << "Cannot make a NURBS from the indicated curve."
      << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::Constructor
//       Access: Public
//  Description: Constructs a NURBS curve according to the indicated
//               NURBS parameters.
////////////////////////////////////////////////////////////////////
NurbsCurve::
NurbsCurve(int order, int num_cvs,
	   const double knots[], const LVecBase4f cvs[]) {
  _order = order;

  int i;
  _cvs.reserve(num_cvs);
  for (i = 0; i < num_cvs; i++) {
    append_cv(cvs[i]);
  }

  int num_knots = num_cvs + order;
  for (i = 0; i < num_knots; i++) {
    set_knot(i, knots[i]);
  }

  recompute();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
NurbsCurve::
~NurbsCurve() {
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::set_order
//       Access: Public, Scheme
//  Description: Changes the order of the curve.  Must be a value from
//               1 to 4.  Can only be done when there are no cv's.
////////////////////////////////////////////////////////////////////
void NurbsCurve::
set_order(int order) {
  if (order < 1 || order > 4) {
    parametrics_cat->warning()
      << "Invalid NURBS curve order: " << order << endl;
    return;
  }
  if (!_cvs.empty()) {
    parametrics_cat->warning()
      << "Cannot change NURBS curve order on a nonempty curve." << endl;
    return;
  }
  _order = order;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::get_order
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
int NurbsCurve::
get_order() const {
  return _order;
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::get_num_cvs
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
int NurbsCurve::
get_num_cvs() const {
  return _cvs.size();
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::insert_cv
//       Access: Public, Scheme
//  Description: Inserts a new CV into the middle of the curve at the
//               indicated parametric value.  This doesn't change the
//               shape or timing of the curve; however, it is
//               irreversible: if the new CV is immediately removed,
//               the curve will be changed.  Returns the index of the
//               newly created CV.
////////////////////////////////////////////////////////////////////
int NurbsCurve::
insert_cv(double t) {
  if (_cvs.empty()) {
    return append_cv(0.0, 0.0, 0.0);
  }

  if (t <= 0) {
    t = 0.0;
  }

  int k = FindCV(t);
  if (k < 0) {
    return append_cv(_cvs.back()._p);
  }

  // Now we are inserting a knot between k-1 and k.  We'll adjust the
  // CV's according to Bohm's rule.

  // First, get the new values of all the CV's that will change.
  // These are the CV's in the range [k - (_order-1), k-1].

  LVecBase4f new_cvs[3];
  int i;
  for (i = 0; i < _order-1; i++) {
    int nk = i + k - (_order-1);
    double ti = GetKnot(nk);
    double d = GetKnot(nk + _order-1) - ti;
    if (d == 0.0) {
      new_cvs[i] = _cvs[nk-1]._p;
    } else {
      double a = (t - ti) / d;
      new_cvs[i] = (1.0-a)*_cvs[nk-1]._p + a*_cvs[nk]._p;
    }
  }

  // Now insert the new CV
  _cvs.insert(_cvs.begin() + k-1, CV());

  // Set all the new position values
  for (i = 0; i < _order-1; i++) {
    int nk = i + k - (_order-1);
    _cvs[nk]._p = new_cvs[i];
  }

  // And set the new knot value.
  _cvs[k-1]._t = t;

  return k-1;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::append_cv
//       Access: Public, Scheme
//  Description: Adds a new CV to the end of the curve.  Creates a new
//               knot value by adding 1 to the last knot value.
//               Returns the index of the new CV.
////////////////////////////////////////////////////////////////////
int NurbsCurve::
append_cv(float x, float y, float z) {
  return append_cv(LVecBase4f(x, y, z, 1.0));
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::remove_cv
//       Access: Public, Scheme
//  Description: Removes the indicated CV from the curve.  Returns
//               true if the CV index was valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
remove_cv(int n) {
  if (n < 0 || n >= (int)_cvs.size()) {
    return false;
  }

  _cvs.erase(_cvs.begin() + n);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::remove_all_cvs
//       Access: Public, Scheme
//  Description: Removes all CV's from the curve.
////////////////////////////////////////////////////////////////////
void NurbsCurve::
remove_all_cvs() {
  _cvs.erase(_cvs.begin(), _cvs.end());
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::set_cv_point
//       Access: Public, Scheme
//  Description: Repositions the indicated CV.  Returns true if there
//               is such a CV, false otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
set_cv_point(int n, float x, float y, float z) {
  if (n < 0 || n >= (int)_cvs.size()) {
    return false;
  }

  float w = _cvs[n]._p[3];
  _cvs[n]._p.set(x*w, y*w, z*w, w);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::get_cv_point
//       Access: Public, Scheme
//  Description: Returns the position of the indicated CV.
////////////////////////////////////////////////////////////////////
void NurbsCurve::
get_cv_point(int n, LVecBase3f &v) const {
  if (n < 0 || n >= (int)_cvs.size()) {
    v = zero;
  } else {
    v = (const LVecBase3f &)_cvs[n]._p / _cvs[n]._p[3];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::get_cv_point
//       Access: Public, Scheme
//  Description: Returns the position of the indicated CV.
////////////////////////////////////////////////////////////////////
const LVecBase3f &NurbsCurve::
get_cv_point(int n) const {
  if (n < 0 || n >= (int)_cvs.size()) {
    return zero;
  } else {
    static LVecBase3f result;
    result = (LVecBase3f &)_cvs[n]._p / _cvs[n]._p[3];
    return result;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::set_cv_weight
//       Access: Public, Scheme
//  Description: Sets the weight of the indicated CV.          
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
set_cv_weight(int n, float w) {
  if (n < 0 || n >= (int)_cvs.size()) {
    return false;
  }

  _cvs[n]._p *= (w / _cvs[n]._p[3]);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::get_cv_weight
//       Access: Public, Scheme
//  Description: Returns the weight of the indicated CV.
////////////////////////////////////////////////////////////////////
float NurbsCurve::
get_cv_weight(int n) const {
  if (n < 0 || n >= (int)_cvs.size()) {
    return 0.0;
  }

  return _cvs[n]._p[3];
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::set_knot
//       Access: Public, Scheme
//  Description: Sets the value of the indicated knot.  There are
//               get_num_cvs()+_order-1 knot values, but the first
//               _order-1 and the last _order-1 knot values cannot be
//               changed.  It is also an error to set a knot value
//               outside the range of its neighbors.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
set_knot(int n, double t) {
  if (n < _order || n-1 >= (int)_cvs.size()) {
    return false;
  }

  _cvs[n-1]._t = t;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::get_knot
//       Access: Public, Scheme
//  Description: Returns the value of the indicated knot.  There are
//               get_num_cvs()+_order-1 knot values.
////////////////////////////////////////////////////////////////////
double NurbsCurve::
get_knot(int n) const {
  return GetKnot(n);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::write
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void NurbsCurve::
write(ostream &out) const {
  switch (get_curve_type()) {
  case PCT_T:
    out << "Time-warping ";
    break;

  case PCT_XYZ:
    out << "XYZ ";
    break;

  case PCT_HPR:
    out << "HPR ";
    break;

  default:
    break;
  }

  out << "NurbsCurve, order " << _order << ", " << get_num_cvs()
      << " CV's.  t ranges from 0 to " << get_max_t() << ".\n";

  out << "CV's:\n";
  int i;
  for (i = 0; i < (int)_cvs.size(); i++) {
    LVecBase3f p = (const LVecBase3f &)_cvs[i]._p / _cvs[i]._p[3];
    out << i << ") " << p << ", weight " << _cvs[i]._p[3] << "\n";
  }

  out << "Knots: ";
  for (i = 0; i < (int)_cvs.size()+_order; i++) {
    out << " " << GetKnot(i);
  }
  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::write_cv
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void NurbsCurve::
write_cv(ostream &out, int n) const {
  if (n < 0 || n >= (int)_cvs.size()) {
    out << "No such CV: " << n << "\n";
  } else {
    LVecBase3f p = (const LVecBase3f &)_cvs[n]._p / _cvs[n]._p[3];
    out << "CV " << n << ": " << p << ", weight " 
	<< _cvs[n]._p[3] << "\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::recompute
//       Access: Public, Scheme
//  Description: Recalculates the curve basis according to the latest
//               position of the CV's, knots, etc.  Until this
//               function is called, adjusting the NURBS parameters
//               will have no visible effect on the curve.  Returns
//               true if the resulting curve is valid, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
recompute() {
  _segs.erase(_segs.begin(), _segs.end());

  double knots[8];
  LVecBase4f cvs[4];

  if ((int)_cvs.size() > _order-1) {
    for (int cv = 0; cv < (int)_cvs.size()-(_order-1); cv++) {
      if (GetKnot(cv+_order-1) < GetKnot(cv+_order)) {
	// There are _order consecutive CV's that define each segment,
	// beginning at cv.  Collect the CV's and knot values that define
	// this segment.
	int c;
	for (c = 0; c < _order; c++) {
	  cvs[c] = _cvs[c+cv]._p;
	}
	for (c = 0; c < _order+_order; c++) {
	  knots[c] = GetKnot(c+cv);
	}
	
	insert_curveseg(_segs.size(), new CubicCurveseg(_order, knots, cvs),
			knots[_order] - knots[_order-1]);
      }
    }
  }

  return !_segs.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::normalize_tlength
//       Access: Public, Scheme
//  Description: Recomputes the knot vector so that the curve is
//               nearly uniformly spaced in parametric time.  That is,
//               calc_length(0, t) == t (approximately) for all t in
//               [0, get_max_t()].  This is only an approximation; its
//               precision depends on the placement of the knots.
////////////////////////////////////////////////////////////////////
void NurbsCurve::
normalize_tlength() {
  int num_knots = _cvs.size() + _order;
  double *new_t = (double *)alloca(sizeof(double) * num_knots);

  int i;
  double last_t = 0.0;
  double last_new_t = 0.0;
  for (i = 0; i < num_knots; i++) {
    double this_t = get_knot(i);

    if (this_t == last_t) {

      // Keep the same knot value.
      new_t[i] = last_new_t;

    } else {
      // Compute a new knot value that represents the distance on the
      // curve since the last knot.
      last_new_t += calc_length(last_t, this_t);
      new_t[i] = last_new_t;
      last_t = this_t;
    }
  }

  // Now set all the knot values at once.
  for (i = 0; i < num_knots; i++) {
    set_knot(i, new_t[i]);
  }
}


#if 0
////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::adjust_pt
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void NurbsCurve::
adjust_pt(double t, 
	  float px, float py, float pz,
	  float tx, float ty, float tz) {
  const ParametricCurve *curve;
  bool result = find_curve(curve, t);

  if (!result) {
    cerr << "  no curve segment at t\n";
    return;
  }

  // Figure out which CV's contributed to this segment.
  int seg = 0;

  dnassert(_cvs.size() > _order-1);

  int cv = 0;
  for (cv = 0; cv < _cvs.size()-(_order-1); cv++) {
    if (GetKnot(cv+_order-1) < GetKnot(cv+_order)) {
      if (seg == _last_ti) {
	break;
      }
      seg++;
    }
  }

  // Now copy the cvs and knots in question.
  double knots[8];
  LVecBase4f cvs[4];

  int c;
  for (c = 0; c < 4; c++) {
    cvs[c] = (c < _order) ? _cvs[c+cv]._p : LVecBase4f(0.0, 0.0, 0.0, 0.0);
  }
  for (c = 0; c < _order+_order; c++) {
    knots[c] = GetKnot(c+cv);
  }

  dnassert(_order>=1 && _order<=4);

  LMatrix4f B;
  compute_nurbs_basis(_order, knots, B);

  LMatrix4f Bi;
  if (!Bi.invertFull(B)) {
    cerr << "Cannot invert B!\n";
    return;
  }

  // We can rebuild a curve segment given four arbitrary properties of
  // the segment: any point along the curve, any tangent along the
  // curve, any control point.  Given any four such properties, a
  // single cubic curve segment is defined.

  // We now want to rebuild the curve segment with the following four
  // properties: the same first control point, the same last control
  // point, and the supplied point and tangent at the indicated value
  // of t.  To do this, we build a matrix T such that:

  // Column 0 of T is the same as column 0 of B^(-1)
  //   This refers to the first control point.
  // Column 1 of T is the vector [ t^3 t^2 t 1 ]
  //   This refers to the point on the curve at t.
  // Column 2 of T is the vector [ 3t^2 2t 1 0 ]
  //   This refers to the tangent to the curve at t.
  // Column 3 of T is the same as column 3 of B^(-1)
  //   This refers to the last control point.

  LMatrix4f T = Bi;
  T.setCol(1, t*t*t, t*t, t, 1.0);
  T.setCol(2, 3.0*t*t, 2.0*t, 1.0, 0.0);

  LMatrix4f Ti;
  if (!Ti.invertFull(T)) {
    cerr << "Cannot invert T!\n";
  }

  // Now we build the matrix P such that P represents the solution of
  // T, above, when T is applied to the geometry and basis matrices.
  // That is, P = G * B * T.

  // Column 0 of P is the first control point.
  // Column 1 of P is the (new) desired point on the curve at t.
  // Column 2 of P is the (new) desired tangent to the curve at t.
  // Column 3 of P is the last control point.

  LMatrix4f P;
  
  P.setCol(0, cvs[0][0], cvs[0][1], cvs[0][2], cvs[0][3]);
  P.setCol(1, px, py, pz, 1.0);
  P.setCol(2, tx, ty, tz, 0.0);
  P.setCol(3, cvs[3][0], cvs[3][1], cvs[3][2], cvs[3][3]);

  // Now we simply solve for G to get G = P * T^(-1) * B^(-1).

  LMatrix4f G = P * Ti * Bi;

  // Now extract the new CV's from the new G matrix, and restore them
  // to the curve.
  for (c = 0; c < _order; c++) {
    LVecBase4f &s = _cvs[c+cv]._p;
    G.getCol(c, &s[0], &s[1], &s[2], &s[3]);
  }
}
#endif 

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::rebuild_curveseg
//       Access: Public, Virtual
//  Description: Rebuilds the current curve segment (as selected by
//               the most recent call to find_curve()) according to
//               the specified properties (see
//               CubicCurveseg::compute_seg).  Returns true if
//               possible, false if something goes horribly wrong.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
rebuild_curveseg(int rtype0, double t0, const LVecBase4f &v0,
		 int rtype1, double t1, const LVecBase4f &v1,
		 int rtype2, double t2, const LVecBase4f &v2,
		 int rtype3, double t3, const LVecBase4f &v3) {
  // Figure out which CV's contributed to this segment.
  int seg = 0;

  nassertr((int)_cvs.size() > _order-1, false);

  int cv = 0;
  for (cv = 0; cv < (int)_cvs.size()-(_order-1); cv++) {
    if (GetKnot(cv+_order-1) < GetKnot(cv+_order)) {
      if (seg == _last_ti) {
	break;
      }
      seg++;
    }
  }

  // Now copy the cvs and knots in question.
  LMatrix4f G;
  double knots[8];

  int c;

  // We only need to build the geometry matrix if at least one of the
  // properties depends on the original value.
  if ((rtype0 | rtype1 | rtype2 | rtype3) & RT_KEEP_ORIG) {
    for (c = 0; c < 4; c++) {
      static const LVecBase4f zero(0.0, 0.0, 0.0, 0.0);
      const LVecBase4f &s = (c < _order) ? _cvs[c+cv]._p : zero;
      
      G.set_col(c, s);
    }
  }

  // But we always need the knot vector to determine the basis matrix.
  for (c = 0; c < _order+_order; c++) {
    knots[c] = GetKnot(c+cv);
  }

  LMatrix4f B;
  compute_nurbs_basis(_order, knots, B);

  LMatrix4f Bi;
  Bi = invert(B);

  if (!CubicCurveseg::compute_seg(rtype0, t0, v0,
				  rtype1, t1, v1,
				  rtype2, t2, v2,
				  rtype3, t3, v3,
				  B, Bi, G)) {
    return false;
  }

  // Now extract the new CV's from the new G matrix, and restore them
  // to the curve.
  for (c = 0; c < _order; c++) {
    _cvs[c+cv]._p = G.get_col(c);
  }

  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::write_egg
//       Access: Public, Scheme
//  Description: Writes an egg description of the nurbs curve to the
//               specified output file.  Creates the file if it does
//               not exist; appends to the end of it if it does.
//               Returns true if the file is successfully written.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
write_egg(const char *filename, CoordinateSystem cs) {
  const char *basename = strrchr(filename, '/');
  basename = (basename==NULL) ? filename : basename+1;

  ofstream out(filename, ios::app);
  return write_egg(out, basename, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::write_egg
//       Access: Public, Scheme
//  Description: Writes an egg description of the nurbs curve to the
//               specified output stream.  Returns true if the file is
//               successfully written.
////////////////////////////////////////////////////////////////////
bool NurbsCurve::
write_egg(ostream &out, const char *basename, CoordinateSystem cs) {
  if (get_name().empty()) {
    // If we don't have a name, come up with one.
    int len = strlen(basename);
    if (len>4 && strcmp(basename+len-4, ".egg")==0) {
      len -= 4;
    }

    char *name = (char *)alloca(len + 5);
    strncpy(name, basename, len);
    switch (_curve_type) {
    case PCT_XYZ:
      strcpy(name+len, "_xyz");
      break;

    case PCT_HPR:
      strcpy(name+len, "_hpr");
      break;

    case PCT_T:
      strcpy(name+len, "_t");
      break;
      
    default:
      name[len] = '\0';
    };

    set_name(name);
  }

  format_egg(out, cs, 0);

  if (out) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::splice
//       Access: Public, Scheme
//  Description: Joins the indicated curve onto the end of this curve,
//               beginning at the indicated time (which must be
//               greater than or equal to max_t).  Normally, the first
//               point of the new curve will be the same as the last
//               point of this curve, but if they are different, the
//               curves will automatically connect; however, the
//               connection may not be smooth and terminal point of
//               the original curve may be lost.
////////////////////////////////////////////////////////////////////
void NurbsCurve::
splice(double t, const NurbsCurve &other) {
  if (other._order != _order) {
    parametrics_cat->warning()
      << "Cannot splice NURBS curves of different orders!" << endl;
    return;
  }

  double old_t = get_max_t();

  if (t < old_t) {
    parametrics_cat->warning()
      << "Invalid splicing in the middle of a curve!" << endl;
    t = old_t;
  }

  // First, define a vector of all the current CV's except the last
  // one.
  vector<CV> new_cvs(_cvs);
  if (!new_cvs.empty()) {
    new_cvs.pop_back();
  }

  // Now add all the new CV's.
  int cv;
  for (cv = 0; cv < (int)other._cvs.size(); cv++) {
    CV new_cv(other._cvs[cv]);

    if (cv+1 < _order) {
      new_cv._t = old_t;
    } else {
      new_cv._t += t;
    }
    new_cvs.push_back(new_cv);
  }

  // Now assign that vector.
  _cvs = new_cvs;

  recompute();
}



////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::format_egg
//       Access: Public
//  Description: Formats the Nurbs curve for output to an Egg file.
////////////////////////////////////////////////////////////////////
void NurbsCurve::
format_egg(ostream &out, CoordinateSystem cs, int indent_level) const {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  if (cs != CS_invalid) {
    indent(out, indent_level)
      << "<CoordinateSystem> { ";
    switch (cs) {
    case CS_zup_right:
      out << "Z-Up";
      break;
      
    case CS_yup_right:
      out << "Y-Up";
      break;
      
    case CS_zup_left:
      out << "Z-Up-Left";
      break;
      
    case CS_yup_left:
      out << "Y-Up-Left";
      break;

    default:
      break;
    }
    out << " }\n\n";
  }

  indent(out, indent_level)
    << "<VertexPool> " << get_name() << ".pool {\n";

  int cv;
  for (cv = 0; cv < (int)_cvs.size(); cv++) {
    indent(out, indent_level+2)
      << "<Vertex> " << cv << " { " << _cvs[cv]._p << " }\n";
  }
  indent(out, indent_level)
    << "}\n";
    
  indent(out, indent_level)
    << "<NURBSCurve> " << get_name() << " {\n";

  if (_curve_type!=PCT_NONE) {
    indent(out, indent_level+2)
      << "<Char*> type { ";
    switch (_curve_type) {
    case PCT_XYZ:
      out << "XYZ";
      break;

    case PCT_HPR:
      out << "HPR";
      break;

    case PCT_T:
      out << "T";
      break;
    };
    out << " }\n";
  }

  indent(out, indent_level+2) << "<Order> { " << _order << " }\n";
  
  indent(out, indent_level+2) << "<Knots> {";
  int k;
  int num_knots = _cvs.size() + _order;

  for (k = 0; k < num_knots; k++) {
    if (k%6 == 0) {
      out << "\n";
      indent(out, indent_level+4);
    }
    out << GetKnot(k) << " ";
  }
  out << "\n";
  indent(out, indent_level+2) << "}\n";

  indent(out, indent_level+2) << "<VertexRef> {";
  for (cv = 0; cv < (int)_cvs.size(); cv++) {
    if (cv%10 == 0) {
      out << "\n";
      indent(out, indent_level+3);
    }
    out << " " << cv;
  }
  out << "\n";
  indent(out, indent_level+4)
    << "<Ref> { " << get_name() << ".pool }\n";
  indent(out, indent_level+2) << "}\n";

  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NurbsCurve::FindCV
//       Access: Protected
//  Description: Finds the first knot whose value is >= t, or -1 if t
//               is beyond the end of the curve.
////////////////////////////////////////////////////////////////////
int NurbsCurve::
FindCV(double t) {
  int i;
  for (i = _order-1; i < (int)_cvs.size(); i++) {
    if (_cvs[i]._t >= t) {
      return i+1;
    }
  }

  return -1;
}

// Filename: hermiteCurve.C
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
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "hermiteCurve.h"
#include "config_parametrics.h"
#include "luse.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
// Statics
////////////////////////////////////////////////////////////////////
TypeHandle HermiteCurve::_type_handle;

static const LVecBase3f zero = LVecBase3f(0.0, 0.0, 0.0);
// This is returned occasionally from some of the functions, and is
// used from time to time as an initializer.


////////////////////////////////////////////////////////////////////
//     Function: Indent
//  Description: This function duplicates a similar function declared
//               in eggBasics.C.  It prints a specified number of
//               spaces to indent each line of output.
////////////////////////////////////////////////////////////////////
static ostream &
Indent(ostream &out, int indent) {
  for (int i=0; i<indent; i++) {
    out << ' ';
  }
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: show_vec3
//  Description: This function writes a LVecBase3f, with a specified
//               number of significant dimensions.
////////////////////////////////////////////////////////////////////
static ostream &
show_vec3(ostream &out, int indent, const LVecBase3f &v, int num_dimensions) {
  Indent(out, indent) << v[0];
  for (int i = 1; i<num_dimensions; i++) {
    out << " " << v[i];
  }
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
HermiteCurveCV::
HermiteCurveCV() {
  _name = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::copy_constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
HermiteCurveCV::
HermiteCurveCV(const HermiteCurveCV &c) :
  _p(c._p), _in(c._in), _out(c._out), 
  _type(c._type) {
    if (c._name==NULL) {
      _name = NULL;
    } else {
      _name = new char[strlen(c._name)+1];
      strcpy(_name, c._name);
    }
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
HermiteCurveCV::
~HermiteCurveCV() {
  if (_name != NULL) {
    delete [] _name;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::set_in
//       Access: Public
//  Description: Sets the CV's in tangent.
////////////////////////////////////////////////////////////////////
void HermiteCurveCV::
set_in(const LVecBase3f &in) {
  _in = in;
  /*
  double l;
  switch (_type) {
  case HC_G1:
    l = _in.length();
    if (l!=0.0) {
      _out = _in * _out.length() / l;
    }
    break;
    
  case HC_SMOOTH:
    _out = _in;
    break;
  }
  */
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::set_out
//       Access: Public
//  Description: Sets the CV's out tangent.
////////////////////////////////////////////////////////////////////
void HermiteCurveCV::
set_out(const LVecBase3f &out) {
  _out = out;
  /*
  double l;
  switch (_type) {
  case HC_G1:
    l = _out.length();
    if (l!=0.0) {
      _in = _out * _in.length() / l;
    }
    break;
    
  case HC_SMOOTH:
    _in = _out;
    break;
  }
  */
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::set_type
//       Access: Public
//  Description: Sets the continuity type of the CV.  Values may be
//               HC_CUT, indicating a discontinous break in the curve,
//               HC_FREE, for unconstrained in and out tangents,
//               HC_G1, for in and out tangents constrained to be
//               collinear, and HC_SMOOTH, for in and out tangents
//               constrained to be equal.  Other than HC_CUT, these
//               are for documentation only; setting this has no
//               direct effect on the tangents.
////////////////////////////////////////////////////////////////////
void HermiteCurveCV::
set_type(int type) {
  _type = type;
  /*
  switch (_type) {
  case HC_G1:
    _out = _out.length() * _in;
    break;
    
  case HC_SMOOTH:
    _out = _in;
    break;
  }
  */
}



////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::set_name
//       Access: Public
//  Description: Sets the name associated with the CV.
////////////////////////////////////////////////////////////////////
void HermiteCurveCV::
set_name(const char *name) {
  if (_name != NULL) {
    delete [] _name;
    _name = NULL;
  }

  if (name != NULL) {
    _name = new char[strlen(name)+1];
    strcpy(_name, name);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurveCV::Output
//       Access: Public
//  Description: Formats the CV for output to an egg file.
////////////////////////////////////////////////////////////////////
void HermiteCurveCV::
Output(ostream &out, int indent, int num_dimensions,
       bool show_in, bool show_out,
       double scale_in, double scale_out) const {
  if (show_in) {
    Indent(out, indent) << "<Vertex> {\n";
    show_vec3(out, indent+2, _p - scale_in * _in / 3.0, 
	      num_dimensions) << "\n";
    Indent(out, indent) << "}\n";
  }

  Indent(out, indent) << "<Vertex> {\n";
  show_vec3(out, indent+2, _p, num_dimensions) << "\n";

  Indent(out, indent+2) << "<Char*> continuity-type { ";
  switch (_type) {
  case HC_CUT:
    out << "Cut";
    break;
    
  case HC_FREE:
    out << "Free";
    break;
    
  case HC_G1:
    out << "G1";
    break;
    
  case HC_SMOOTH:
    out << "Smooth";
    break;
  };
  out << " }\n";

  Indent(out, indent) << "}\n";

  if (show_out) {
    Indent(out, indent) << "<Vertex> {\n";
    show_vec3(out, indent+2, _p + scale_out * _out / 3.0, 
	      num_dimensions) << "\n";
    Indent(out, indent) << "}\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::Constructor
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
HermiteCurve::
HermiteCurve() {
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::Constructor
//       Access: Public, Scheme
//  Description: Constructs a Hermite from the indicated (possibly
//               non-hermite) curve.
////////////////////////////////////////////////////////////////////
HermiteCurve::
HermiteCurve(const ParametricCurve &nc) {
  if (!nc.convert_to_hermite(*this)) {
    parametrics_cat->warning()
      << "Cannot make a Hermite from the indicated curve."
      << endl;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
HermiteCurve::
~HermiteCurve() {
}






////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_num_cvs
//       Access: Public, Scheme
//  Description: Returns the number of CV's in the curve.
////////////////////////////////////////////////////////////////////
int HermiteCurve::
get_num_cvs() const {
  return _points.size();
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::insert_cv
//       Access: Public, Scheme
//  Description: Inserts a new CV at the given parametric point along
//               the curve.  If this parametric point is already on
//               the curve, the CV is assigned an index between its
//               two neighbors and the indices of all following CV's
//               are incremented by 1; its in and out tangents are
//               chosen to keep the curve consistent.  If the new
//               parametric point is beyond the end of the existing
//               curve, the curve is extended to meet it and the new
//               CV's position, in tangent, and out tangent are set to
//               zero.
//
//               The index number of the new CV is returned.
////////////////////////////////////////////////////////////////////
int HermiteCurve::
insert_cv(double t) {
  if (!is_valid() || t >= get_max_t()) {
    int n = append_cv(HC_SMOOTH, 0.0, 0.0, 0.0);
    set_cv_tstart(n, t);
    return n;
  }

  t = min(max(t, 0.0), get_max_t());

  int n = find_cv(t);
  nassertr(n+1<get_num_cvs(), 0);

  HermiteCurveCV cv;
  LVecBase3f tan;
  cv._type = HC_SMOOTH;
  get_pt(t, cv._p, tan);
  cv._out = cv._in = tan / 2.0;

  _points.insert(_points.begin() + n + 1, cv);
  bool result =
    insert_curveseg(n, new CubicCurveseg, t - get_cv_tstart(n));
  nassertr(result, 0);

  recompute_basis();
  invalidate_all();

  return n + 1;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::append_cv
//       Access: Public, Scheme
//  Description: Adds a new CV to the end of the curve.  The new CV is
//               given initial in/out tangents of 0.  The return value
//               is the index of the new CV.
////////////////////////////////////////////////////////////////////
int HermiteCurve::
append_cv(int type, float x, float y, float z) {
  HermiteCurveCV cv;
  cv.set_type(type);
  cv.set_point(LVecBase3f(x, y, z));
  cv.set_in(zero);
  cv.set_out(zero);
  _points.push_back(cv);
  if (_points.size()>1) {
    bool result =
      insert_curveseg(_segs.size(), new CubicCurveseg, 1.0);
    nassertr(result, 0);
  }

  recompute_basis();
  invalidate_all();

  return _points.size()-1;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::remove_cv
//       Access: Public, Scheme
//  Description: Removes the given CV from the curve.  Returns true if
//               the CV existed, false otherwise.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
remove_cv(int n) {
  if (n < 0 || n >= (int)_points.size()) {
    return false;
  }

  _points.erase(_points.begin() + n);
  if (_segs.size()>0) {
    remove_curveseg(_segs.size()-1);
  }
  
  recompute_basis();
  invalidate_all();
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::remove_all_cvs
//       Access: Public, Scheme
//  Description: Removes all CV's from the curve.
////////////////////////////////////////////////////////////////////
void HermiteCurve::
remove_all_cvs() {
  _points.erase(_points.begin(), _points.end());
  remove_all_curvesegs();
  
  invalidate_all();
}




////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::set_cv_type
//       Access: Public, Scheme
//  Description: Changes the given CV's continuity type.  Legal values
//               are HC_CUT, HC_FREE, HC_G1, or HC_SMOOTH.
//
//               Other than HC_CUT, these have no effect on the actual
//               curve; it remains up to user software to impose the
//               constraints these imply.
//
//               HC_CUT implies a disconnection of the curve; HC_FREE
//               imposes no constraints on the tangents; HC_G1
//               forces the tangents to be collinear, and HC_SMOOTH
//               forces the tangents to be identical.  Setting type
//               type to HC_G1 or HC_SMOOTH may adjust the out
//               tangent to match the in tangent.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
set_cv_type(int n, int type) {
  if (n < 0 || n >= (int)_points.size()) {
    return false;
  }

  bool changed_cut = false;

  if (type!=_points[n]._type) {
    changed_cut = (type==HC_CUT || _points[n]._type==HC_CUT);
    _points[n].set_type(type);
  }

  invalidate_cv(n, changed_cut);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::set_cv_point
//       Access: Public, Scheme
//  Description: Changes the given CV's position.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
set_cv_point(int n, float x, float y, float z) {
  if (n < 0 || n >= (int)_points.size()) {
    return false;
  }
  _points[n].set_point(LVecBase3f(x, y, z));
  invalidate_cv(n, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::set_cv_in
//       Access: Public, Scheme
//  Description: Changes the given CV's in tangent.  Depending on the
//               continuity type, this may also adjust the out
//               tangent.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
set_cv_in(int n, float x, float y, float z) {
  if (n < 0 || n >= (int)_points.size()) {
    return false;
  }
  _points[n].set_in(LVecBase3f(x, y, z));
  invalidate_cv(n, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::set_cv_out
//       Access: Public, Scheme
//  Description: Changes the given CV's out tangent.  Depending on the
//               continuity type, this may also adjust the in
//               tangent.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
set_cv_out(int n, float x, float y, float z) {
  if (n < 0 || n >= (int)_points.size()) {
    return false;
  }
  _points[n].set_out(LVecBase3f(x, y, z));
  invalidate_cv(n, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::set_cv_tstart
//       Access: Public, Scheme
//  Description: Changes the given CV's parametric starting time.
//               This may affect the shape of the curve.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
set_cv_tstart(int n, double tstart) {
  if (n <= 0 || n >= (int)_points.size()) {
    return false;
  }
  if (fabs(tstart - get_cv_tstart(n)) > 0.0001) {
    set_tlength(n-1, tstart - get_tstart(n-1));
    recompute_basis();
    invalidate_all();
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::set_cv_name
//       Access: Public, Scheme
//  Description: Changes the name associated with a particular CV.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
set_cv_name(int n, const char *name) {
  if (n < 0 || n >= (int)_points.size()) {
    return false;
  }
  _points[n].set_name(name);
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_cv_type
//       Access: Public, Scheme
//  Description: Returns the given CV's continuity type, HC_CUT,
//               HC_FREE, HC_G1, or HC_SMOOTH, or 0 if there is
//               no such CV.
////////////////////////////////////////////////////////////////////
int HermiteCurve::
get_cv_type(int n) const {
  if (n < 0 || n >= (int)_points.size()) {
    return 0;
  }

  return _points[n]._type;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_cv_point
//       Access: Public, Scheme
//  Description: Returns the position of the given CV.
////////////////////////////////////////////////////////////////////
const LVecBase3f &HermiteCurve::
get_cv_point(int n) const {
  if (n < 0 || n >= (int)_points.size()) {
    return zero;
  }

  return _points[n]._p;
}
void HermiteCurve::
get_cv_point(int n, LVecBase3f &v) const {
  v = get_cv_point(n);
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_cv_in
//       Access: Public, Scheme
//  Description: Returns the in tangent of the given CV.
////////////////////////////////////////////////////////////////////
const LVecBase3f &HermiteCurve::
get_cv_in(int n) const {
  if (n < 0 || n >= (int)_points.size() || _points[n-1]._type==HC_CUT) {
    return zero;
  }

  return _points[n]._in;
}
void HermiteCurve::
get_cv_in(int n, LVecBase3f &v) const {
  v = get_cv_in(n);
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_cv_out
//       Access: Public, Scheme
//  Description: Returns the out tangent of the given CV.
////////////////////////////////////////////////////////////////////
const LVecBase3f &HermiteCurve::
get_cv_out(int n) const {
  if (n < 0 || n >= (int)_points.size() || _points[n]._type==HC_CUT) {
    return zero;
  }

  return _points[n]._out;
}
void HermiteCurve::
get_cv_out(int n, LVecBase3f &v) const {
  v = get_cv_out(n);
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_cv_tstart
//       Access: Public, Scheme
//  Description: Returns the starting point in parametric space of the
//               given CV.
////////////////////////////////////////////////////////////////////
double HermiteCurve::
get_cv_tstart(int n) const {
  if (n<0) {
    return 0.0;
  } else if (n >= (int)_points.size()) {
    return get_max_t();
  }

  return get_tstart(n);
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::get_cv_name
//       Access: Public, Scheme
//  Description: Returns the name of the given CV, or NULL.
////////////////////////////////////////////////////////////////////
const char *HermiteCurve::
get_cv_name(int n) const {
  if (n < 0 || n >= (int)_points.size()) {
    return NULL;
  }

  return _points[n]._name;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::Print
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void HermiteCurve::
Print() const {
  ostream& out = parametrics_cat->info();

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

  out
    << "HermiteCurve, " << get_num_cvs() << " CV's.  t ranges from 0 to "
    << get_max_t()
    << endl;
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::print_cv
//       Access: Public, Scheme
//  Description: 
////////////////////////////////////////////////////////////////////
void HermiteCurve::
print_cv(int n) const {
  ostream& out = parametrics_cat->info();
  out << "CV";
  if (get_cv_name(n)!=NULL) {
    out << " " << get_cv_name(n);
  }

  out << " at t = " << get_cv_tstart(n)
    << "\npoint = " << get_cv_point(n)
    << "\nin = " << get_cv_in(n) << " out = " << get_cv_out(n)
    << "\ncontinuity type = ";

  switch (get_cv_type(n)) {
  case HC_CUT:
    out << "Cut";
    break;

  case HC_FREE:
    out << "Free";
    break;

  case HC_G1:
    out << "G1";
    break;

  case HC_SMOOTH:
    out << "Smooth";
    break;

  default:
    break;
  }

  out << "\n" << endl;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::write_egg
//       Access: Public, Scheme
//  Description: Writes an egg description of the hermite curve to the
//               specified output file.  Creates the file if it does
//               not exist; appends to the end of it if it does.
//               Returns true if the file is successfully written.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
write_egg(const char *filename) {
  const char *basename = strrchr(filename, '/');
  basename = (basename==NULL) ? filename : basename+1;

  ofstream out(filename, ios::app);
  return write_egg(out, basename);
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::write_egg
//       Access: Public, Scheme
//  Description: Writes an egg description of the hermite curve to the
//               specified output stream.  Returns true if the file is
//               successfully written.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
write_egg(ostream &out, const char *basename) {
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

  Output(out);

  if (out) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::rebuild_curveseg
//       Access: Public, Virtual
//  Description: Rebuilds the current curve segment (as selected by
//               the most recent call to find_curve()) according to
//               the specified properties (see
//               CubicCurveseg::compute_seg).  Returns true if
//               possible, false if something goes horribly wrong.
////////////////////////////////////////////////////////////////////
bool HermiteCurve::
rebuild_curveseg(int, double, const LVecBase4f &,
		 int, double, const LVecBase4f &,
		 int, double, const LVecBase4f &,
		 int, double, const LVecBase4f &) {
  cerr << "rebuild_curveseg not implemented for this curve type.\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::Output
//       Access: Public
//  Description: Formats the Hermite curve for output to an Egg file.
////////////////////////////////////////////////////////////////////
void HermiteCurve::
Output(ostream &out, int indent) const {
  Indent(out, indent)
    << "<VertexPool> " << get_name() << ".pool {\n";

  int i;
  for (i = 0; i < (int)_points.size(); i++) {
    bool show_in = (i != 0);
    bool show_out = (i != (int)_points.size()-1);
    _points[i].Output(out, indent+2, _num_dimensions, 
		      show_in, show_out,
		      show_in ? get_tlength(i-1) : 0.0,
		      show_out ? get_tlength(i) : 0.0);
  }
  Indent(out, indent) << "}\n";
    
  Indent(out, indent) << "<BezierCurve> " << get_name() << " {\n";

  if (_curve_type!=PCT_NONE) {
    Indent(out, indent+2) << "<Char*> type { ";
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

  Indent(out, indent+2) << "<TLengths> {";
  if (_points.size() > 1) {
    for (i = 0; i < (int)_segs.size(); i++) {
      if (i%10 == 1) {
	out << "\n";
	Indent(out, indent+3);
      }
      out << " " << get_tlength(i);
    }
  }
  out << "\n";
  Indent(out, indent+2) << "}\n";

  Indent(out, indent+2) << "<VertexRef> {";
  for (i = 1; i <= (int)_points.size() * 3 - 2; i++) {
    if (i%10 == 1) {
      out << "\n";
      Indent(out, indent+3);
    }
    out << " " << i;
  }
  out << "\n";
  Indent(out, indent+4) << "<Ref> { " << get_name() << ".pool }\n";
  Indent(out, indent+2) << "}\n";

  Indent(out, indent) << "}\n";
}


static void
wrap_hpr(const LVecBase3f &hpr1, LVecBase3f &hpr2) {
  for (int i = 0; i < 3; i++) {
    while ((hpr2[i] - hpr1[i]) > 180.0) {
      hpr2[i] -= 360.0;
    }
    
    while ((hpr2[i] - hpr1[i]) < -180.0) {
      hpr2[i] += 360.0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::invalidate_cv
//       Access: Protected
//  Description: Recomputes the CV and its neighbors appropriately
//               after a change.
////////////////////////////////////////////////////////////////////
void HermiteCurve::
invalidate_cv(int n, bool redo_all) {
  double t1 = 0.0, t2 = get_max_t();
  if (n>0 && _points[n-1]._type!=HC_CUT) {
    const HermiteCurveCV &p1 = _points[n-1];
    HermiteCurveCV p2(_points[n]);
    if (_curve_type == PCT_HPR) {
      wrap_hpr(p1._p, p2._p);
    }
    get_curveseg(n-1)->hermite_basis(p1, p2, get_tlength(n-1));
    t1 = get_cv_tstart(n-1);
  }

  if (n+1 < (int)_points.size()) {
    if (_points[n]._type==HC_CUT) {
      BezierSeg seg;
      seg._v[0] = seg._v[1] = seg._v[2] = seg._v[3] = _points[n]._p;
      get_curveseg(n)->bezier_basis(seg);
    } else {
      const HermiteCurveCV &p1 = _points[n];
      HermiteCurveCV p2(_points[n+1]);
      if (_curve_type == PCT_HPR) {
	wrap_hpr(p1._p, p2._p);
      }
      get_curveseg(n)->hermite_basis(p1, p2, get_tlength(n));
      t2 = get_cv_tstart(n+2);
    }
  }

  if (is_valid()) {
    if (redo_all) {
      invalidate_all();
    } else {
      invalidate(t1, t2);
    }
  }
}



////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::find_cv
//       Access: Protected
//  Description: Finds the CV immediately preceding the given value of
//               t.
////////////////////////////////////////////////////////////////////
int HermiteCurve::
find_cv(double t) {
  nassertr(is_valid(), 0);

  int n;
  for (n = 0; n < (int)_segs.size(); n++) {
    if (_segs[n]._tend+0.00001 > t) {
      break;
    }
  }

  return n;
}


////////////////////////////////////////////////////////////////////
//     Function: HermiteCurve::recompute_basis
//       Access: Protected
//  Description: Recomputes the coefficients for all the CV's in the
//               curve.  This is intended to be called whenever the
//               CV's have been changed in some drastic way, and it's
//               safest just to recompute everything.
////////////////////////////////////////////////////////////////////
void HermiteCurve::
recompute_basis() {
  int n;
  for (n = 0; n < (int)_segs.size(); n++) {
    if (_points[n]._type==HC_CUT) {
      BezierSeg seg;
      seg._v[0] = seg._v[1] = seg._v[2] = seg._v[3] = _points[n]._p;
      get_curveseg(n)->bezier_basis(seg);
    } else {
      const HermiteCurveCV &p1 = _points[n];
      HermiteCurveCV p2(_points[n+1]);
      if (_curve_type == PCT_HPR) {
	wrap_hpr(p1._p, p2._p);
      }
      get_curveseg(n)->hermite_basis(p1, p2, get_tlength(n));
    }
  }
}


// Filename: eggCurve.cxx
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggCurve.h"

#include <string_utils.h>
#include <notify.h>

TypeHandle EggCurve::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggCurve::string_curve_type
//       Access: Public, Static
//  Description: Returns the CurveType value associated with the given
//               string representation, or CT_invalid if the string
//               does not match any known CurveType value.
////////////////////////////////////////////////////////////////////
EggCurve::CurveType EggCurve::
string_curve_type(const string &string) {
  if (cmp_nocase_uh(string, "xyz") == 0) {
    return CT_xyz;
  } else if (cmp_nocase_uh(string, "hpr") == 0) {
    return CT_hpr;
  } else if (cmp_nocase_uh(string, "t") == 0) {
    return CT_t;
  } else {
    return CT_none;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveType output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggCurve::CurveType t) {
  switch (t) {
  case EggCurve::CT_none:
    return out << "none";
  case EggCurve::CT_xyz:
    return out << "XYZ";
  case EggCurve::CT_hpr:
    return out << "HPR";
  case EggCurve::CT_t:
    return out << "T";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}


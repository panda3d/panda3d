// Filename: eggAlphaMode.cxx
// Created by:  drose (20Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggAlphaMode.h"
#include <indent.h>
#include <string_utils.h>
#include <notify.h>

TypeHandle EggAlphaMode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::write
//       Access: Public
//  Description: Writes the attributes to the indicated output stream in
//               Egg format.
////////////////////////////////////////////////////////////////////
void EggAlphaMode::
write(ostream &out, int indent_level) const {
  if (get_alpha_mode() != AM_unspecified) {
    indent(out, indent_level)
      << "<Scalar> alpha { " << get_alpha_mode() << " }\n";
  }
  if (has_draw_order()) {
    indent(out, indent_level)
      << "<Scalar> draw-order { " << get_draw_order() << " }\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggAlphaMode::
operator == (const EggAlphaMode &other) const {
  if (_alpha_mode != other._alpha_mode ||
      _has_draw_order != other._has_draw_order) {
    return false;
  }

  if (_has_draw_order) {
    if (_draw_order != other._draw_order) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggAlphaMode::
operator < (const EggAlphaMode &other) const {
  if (_alpha_mode != other._alpha_mode) {
    return (int)_alpha_mode < (int)other._alpha_mode;
  }

  if (_has_draw_order != other._has_draw_order) {
    return (int)_has_draw_order < (int)other._has_draw_order;
  }

  if (_has_draw_order) {
    if (_draw_order != other._draw_order) {
      return _draw_order < other._draw_order;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggAlphaMode::string_alpha_mode
//       Access: Public
//  Description: Returns the AlphaMode value associated with the given
//               string representation, or AM_unspecified if the string
//               does not match any known AlphaMode value.
////////////////////////////////////////////////////////////////////
EggAlphaMode::AlphaMode EggAlphaMode::
string_alpha_mode(const string &string) {
  if (cmp_nocase_uh(string, "off") == 0) {
    return AM_off;
  } else if (cmp_nocase_uh(string, "on") == 0) {
    return AM_on;
  } else if (cmp_nocase_uh(string, "blend") == 0) {
    return AM_blend;
  } else if (cmp_nocase_uh(string, "blend_no_occlude") == 0) {
    return AM_blend_no_occlude;
  } else if (cmp_nocase_uh(string, "ms") == 0) {
    return AM_ms;
  } else if (cmp_nocase_uh(string, "ms_mask") == 0) {
    return AM_ms_mask;
  } else {
    return AM_unspecified;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AlphaMode output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggAlphaMode::AlphaMode mode) {
  switch (mode) {
  case EggAlphaMode::AM_unspecified:
    return out << "unspecified";
  case EggAlphaMode::AM_off:
    return out << "off";
  case EggAlphaMode::AM_on:
    return out << "on";
  case EggAlphaMode::AM_blend:
    return out << "blend";
  case EggAlphaMode::AM_blend_no_occlude:
    return out << "blend_no_occlude";
  case EggAlphaMode::AM_ms:
    return out << "ms";
  case EggAlphaMode::AM_ms_mask:
    return out << "ms_mask";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

    

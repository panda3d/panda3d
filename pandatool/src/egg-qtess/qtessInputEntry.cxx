/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessInputEntry.cxx
 * @author drose
 * @date 2003-10-13
 */

#include "qtessInputEntry.h"
#include "qtessSurface.h"
#include "qtessGlobals.h"
#include "config_egg_qtess.h"
#include "indent.h"
#include "string_utils.h"

#include <ctype.h>
#include <algorithm>

using std::string;

/**
 *
 */
QtessInputEntry::
QtessInputEntry(const string &name) {
  _type = T_undefined;
  _num_patches = 0.0;
  _auto_place = QtessGlobals::_auto_place;
  _auto_distribute = QtessGlobals::_auto_distribute;
  _curvature_ratio = QtessGlobals::_curvature_ratio;
  if (!name.empty()) {
    add_node_name(name);
  }
}

/**
 *
 */
void QtessInputEntry::
operator = (const QtessInputEntry &copy) {
  _node_names = copy._node_names;
  _type = copy._type;
  _num_tris = copy._num_tris;
  _num_u = copy._num_u;
  _num_v = copy._num_v;
  _per_isoparam = copy._per_isoparam;
  _iso_u = copy._iso_u;
  _iso_v = copy._iso_v;
  _surfaces = copy._surfaces;
  _num_patches = copy._num_patches;
  _auto_place = copy._auto_place;
  _auto_distribute = copy._auto_distribute;
  _curvature_ratio = copy._curvature_ratio;
  _importance = copy._importance;
  _constrain_u = copy._constrain_u;
  _constrain_v = copy._constrain_v;
}

/**
 * An STL function object to determine if two doubles are very nearly equal.
 * Used in set_uv(), below.
 */
class DoublesAlmostEqual {
public:
  int operator ()(double a, double b) const {
    return fabs(a - b) < 0.00001;
  }
};

/**
 * An STL function object to determine if a double is vert nearly equal the
 * supplied value .  Used in set_uv(), below.
 */
class DoubleAlmostMatches {
public:
  DoubleAlmostMatches(double v) : _v(v) {}
  int operator ()(double a) const {
    return fabs(a - _v) < 0.00001;
  }
  double _v;
};


/**
 * Sets specific tesselation.  The tesselation will be u by v quads, with the
 * addition of any isoparams described in the list of params.
 */
void QtessInputEntry::
set_uv(int u, int v, const string params[], int num_params) {
  _num_u = u;
  _num_v = v;

  // First, fill up the arrays with the defaults.
  int i;
  for (i = 0; i <= _num_u; i++) {
    _iso_u.push_back(i);
  }
  for (i = 0; i <= _num_v; i++) {
    _iso_v.push_back(i);
  }

  // Then get out all the additional entries.
  for (i = 0; i < num_params; i++) {
    const string &param = params[i];

    if (param[0] == '!' && param.size() > 2) {
      double value;
      if (!string_to_double(param.substr(2), value)) {
        qtess_cat.warning()
          << "Ignoring invalid parameter: " << param << "\n";
      } else {
        switch (tolower(param[1])) {
        case 'u':
          _auto_place = false;
          _iso_u.erase(remove_if(_iso_u.begin(), _iso_u.end(),
                                 DoubleAlmostMatches(value)),
                       _iso_u.end());
          break;

        case 'v':
          _auto_place = false;
          _iso_v.erase(remove_if(_iso_v.begin(), _iso_v.end(),
                                 DoubleAlmostMatches(value)),
                       _iso_v.end());
          break;

        default:
          qtess_cat.warning()
            << "Ignoring invalid parameter: " << params[i] << "\n";
        }
      }
    } else {
      double value;
      if (!string_to_double(param.substr(1), value)) {
        qtess_cat.warning()
          << "Ignoring invalid parameter: " << param << "\n";
      } else {
        switch (tolower(param[0])) {
        case 'u':
          _auto_place = false;
          _iso_u.push_back(value);
          break;

        case 'v':
          _auto_place = false;
          _iso_v.push_back(value);
          break;

        default:
          qtess_cat.warning()
            << "Ignoring invalid parameter: " << params[i] << "\n";
        }
      }
    }
  }

  // Now sort them into ascending order and remove duplicates.
  sort(_iso_u.begin(), _iso_u.end());
  sort(_iso_v.begin(), _iso_v.end());
  _iso_u.erase(unique(_iso_u.begin(), _iso_u.end(), DoublesAlmostEqual()), _iso_u.end());
  _iso_v.erase(unique(_iso_v.begin(), _iso_v.end(), DoublesAlmostEqual()), _iso_v.end());

  _type = T_uv;
}


/**
 * May be called a number of times before set_uv() to add specific additional
 * isoparams to the tesselation.
 */
void QtessInputEntry::
add_extra_u_isoparam(double u) {
  _iso_u.push_back(u);
}

/**
 * May be called a number of times before set_uv() to add specific additional
 * isoparams to the tesselation.
 */
void QtessInputEntry::
add_extra_v_isoparam(double v) {
  _iso_v.push_back(v);
}

/**
 * Tests the surface to see if it matches any of the regular expressions that
 * define this node entry.  If so, adds it to the set of matched surfaces and
 * returns the type of the matching entry.  If no match is found, returns
 * T_undefined.
 */
QtessInputEntry::Type QtessInputEntry::
match(QtessSurface *surface) {
  const string &name = surface->get_name();

  NodeNames::const_iterator nni;
  for (nni = _node_names.begin();
       nni != _node_names.end();
       ++nni) {
    const GlobPattern &pattern = (*nni);
    if (pattern.matches(name)) {
      // We have a winner!
      switch (_type) {
      case T_importance:
        // A type of "Importance" is a special case.  This entry doesn't
        // specify any kind of tesselation on the surface, and in fact doesn't
        // preclude the surface from matching anything later.  It just
        // specifies the relative importance of the surface to all the other
        // surfaces.
        if (qtess_cat.is_debug()) {
          qtess_cat.debug()
            << "Assigning importance of " << _importance*100.0
            << "% to " << name << "\n";
        }
        surface->set_importance(_importance);
        return T_undefined;

      case T_match_uu:
      case T_match_uv:
        // Similarly for type "matchUU".  This indicates that all the surfaces
        // that match this one must all share the U-tesselation with whichever
        // surface first matched against the first node name.
        if (nni == _node_names.begin() && _constrain_u==nullptr) {
          // This is the lucky surface that dominates!
          _constrain_u = surface;
        } else {
          if (_type == T_match_uu) {
            surface->set_match_u(&_constrain_u, true);
          } else {
            surface->set_match_v(&_constrain_u, false);
          }
        }
        return T_undefined;

      case T_match_vv:
      case T_match_vu:
        // Ditto for "matchVV".
        if (nni == _node_names.begin() && _constrain_v==nullptr) {
          // This is the lucky surface that dominates!
          _constrain_v = surface;
        } else {
          if (_type == T_match_vv) {
            surface->set_match_v(&_constrain_v, true);
          } else {
            surface->set_match_u(&_constrain_v, false);
          }
        }
        return T_undefined;

      case T_min_u:
        // And for min U and V.
        if (qtess_cat.is_debug()) {
          qtess_cat.debug()
            << "Assigning minimum of " << _num_u << " in U to "
            << name << "\n";
        }
        surface->set_min_u(_num_u);
        return T_undefined;

      case T_min_v:
        if (qtess_cat.is_debug()) {
          qtess_cat.debug()
            << "Assigning minimum of " << _num_v << " in V to "
            << name << "\n";
        }
        surface->set_min_v(_num_v);
        return T_undefined;

      default:
        _surfaces.push_back(surface);
        if (_auto_distribute) {
          _num_patches += surface->get_score(_curvature_ratio);
        } else {
          _num_patches += surface->count_patches();
        }
        return _type;
      }
    }
  }

  return T_undefined;
}

/**
 * Determines the tesselation u,v amounts of each attached surface, and stores
 * this information in the surface pointer.  Returns the total number of tris
 * that will be produced.
 */
int QtessInputEntry::
count_tris(double tri_factor, int attempts) {
  int total_tris = 0;
  bool aim_for_tris = false;

  if (_type == T_num_tris && _num_patches > 0.0) {
    // If we wanted to aim for a particular number of triangles for the group,
    // choose a per-isoparam setting that will approximately achieve this.
    if (_auto_distribute) {
      set_per_score(sqrt(0.5 * (double)_num_tris / _num_patches / tri_factor));
    } else {
      set_per_isoparam(sqrt(0.5 * (double)_num_tris / _num_patches / tri_factor));
    }
    aim_for_tris = true;
  }

  Surfaces::iterator si;
  for (si = _surfaces.begin(); si != _surfaces.end(); ++si) {
    QtessSurface *surface = (*si);

    switch (_type) {
    case T_undefined:
    case T_omit:
      surface->omit();
      break;

    case T_uv:
      if (!_iso_u.empty() && !_iso_v.empty() && !_auto_place) {
        surface->tesselate_specific(_iso_u, _iso_v);
      } else {
        surface->tesselate_uv(_num_u, _num_v, _auto_place, _curvature_ratio);
      }
      break;

    case T_per_isoparam:
      surface->tesselate_per_isoparam(_per_isoparam, _auto_place, _curvature_ratio);
      break;

    case T_per_score:
      surface->tesselate_per_score(_per_isoparam, _auto_place, _curvature_ratio);
      break;

    default:
      break;
    }

    total_tris += surface->count_tris();
  }

  if (aim_for_tris && attempts < 10 &&
      (double)total_tris / (double)_num_tris > 1.1) {
    // We'd like to get within 10% of the requested number of triangles, if
    // possible.  Keep trying until we do, or until we just need to give up.
    set_num_tris(_num_tris);
    return count_tris(tri_factor * total_tris / _num_tris, attempts + 1);
  }

  return total_tris;
}


/**
 * This function is used to identify the extra isoparams in the list added by
 * user control.
 */
void QtessInputEntry::
output_extra(std::ostream &out, const pvector<double> &iso, char axis) {
  pvector<double>::const_iterator di;
  int expect = 0;
  for (di = iso.begin(); di != iso.end(); ++di) {
    while ((*di) > (double)expect) {
      // Didn't find one we were expecting.  Omit it.
      out << " !" << axis << expect;
    }
    if ((*di)==(double)expect) {
      // Here's one we were expecting; ignore it.
      expect++;
    } else {
      // Here's a new one.  Write it.
      out << " " << axis << *di;
    }
  }
}

/**
 *
 */
void QtessInputEntry::
output(std::ostream &out) const {
  NodeNames::const_iterator nni;
  for (nni = _node_names.begin();
       nni != _node_names.end();
       ++nni) {
    out << (*nni) << " ";
  }
  out << ": ";

  bool show_auto = false;

  switch (_type) {
  case T_undefined:
    break;

  case T_omit:
    out << "omit";
    break;

  case T_num_tris:
    out << _num_tris;
    show_auto = true;
    break;

  case T_uv:
    out << _num_u << " " << _num_v;
    output_extra(out, _iso_u, 'u');
    output_extra(out, _iso_v, 'v');
    show_auto = true;
    break;

  case T_per_isoparam:
  case T_per_score:
    out << "i" << _per_isoparam;
    show_auto = true;
    break;

  case T_importance:
    out << _importance * 100.0 << "%";
    break;

  case T_match_uu:
    out << "matchuu";
    break;

  case T_match_vv:
    out << "matchvv";
    break;

  case T_match_uv:
    out << "matchuv";
    break;

  case T_match_vu:
    out << "matchvu";
    break;

  case T_min_u:
    out << "minu " << _num_u;
    break;

  case T_min_v:
    out << "minv " << _num_v;
    break;

  default:
    out << "Invalid!";
  }

  if (show_auto) {
    out << " " << (_auto_place?"":"!") << "ap"
        << " " << (_auto_distribute?"":"!") << "ad";
    if (_auto_place || _auto_distribute) {
      out << " ar" << _curvature_ratio;
    }
  }
}

/**
 *
 */
void QtessInputEntry::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
}

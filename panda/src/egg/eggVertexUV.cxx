// Filename: eggVertexUV.cxx
// Created by:  drose (20Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "eggVertexUV.h"
#include "eggParameters.h"

#include "indent.h"

TypeHandle EggVertexUV::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexUV::
EggVertexUV(const string &name, const LTexCoordd &uv) :
  EggNamedObject(name),
  _flags(0),
  _uvw(uv[0], uv[1], 0.0)
{
  if (get_name() == "default") {
    clear_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexUV::
EggVertexUV(const string &name, const LTexCoord3d &uvw) :
  EggNamedObject(name),
  _flags(F_has_w),
  _uvw(uvw)
{
  if (get_name() == "default") {
    clear_name();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexUV::
EggVertexUV(const EggVertexUV &copy) :
  EggNamedObject(copy),
  _duvs(copy._duvs),
  _flags(copy._flags),
  _tangent(copy._tangent),
  _binormal(copy._binormal),
  _uvw(copy._uvw)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexUV &EggVertexUV::
operator = (const EggVertexUV &copy) {
  EggNamedObject::operator = (copy);
  _duvs = copy._duvs;
  _flags = copy._flags;
  _tangent = copy._tangent;
  _binormal = copy._binormal;
  _uvw = copy._uvw;

  return (*this);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggVertexUV::
~EggVertexUV() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::make_average
//       Access: Published, Static
//  Description: Creates a new EggVertexUV that contains the
//               averaged values of the two given objects.  It is
//               an error if they don't have the same name.
///////////////////////////////////////////////////////////////////
PT(EggVertexUV) EggVertexUV::
make_average(const EggVertexUV *first, const EggVertexUV *second) {
  nassertr(first->get_name() == second->get_name(), NULL);
  int flags = first->_flags & second->_flags;

  LTexCoord3d uvw = (first->_uvw + second->_uvw) / 2;

  PT(EggVertexUV) new_obj = new EggVertexUV(first->get_name(), uvw);
  new_obj->_flags = flags;
  new_obj->_tangent = (first->_tangent + second->_tangent) / 2;
  new_obj->_binormal = (first->_binormal + second->_binormal) / 2;

  // Normalize because we're polite.
  new_obj->_tangent.normalize();
  new_obj->_binormal.normalize();
  return new_obj;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::transform
//       Access: Published, Virtual
//  Description: Applies the indicated transformation matrix to the
//               UV's tangent and/or binormal.  This does nothing if
//               there is no tangent or binormal.
////////////////////////////////////////////////////////////////////
void EggVertexUV::
transform(const LMatrix4d &mat) {
  if (has_tangent()) {
    _tangent = _tangent * mat;
    _tangent.normalize();
  }
  if (has_binormal()) {
    _binormal = _binormal * mat;
    _binormal.normalize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggVertexUV::
write(ostream &out, int indent_level) const {
  string inline_name = get_name();
  if (!inline_name.empty()) {
    inline_name += ' ';
  }

  if (_duvs.empty() && (_flags & ~F_has_w) == 0) {
    if (has_w()) {
      indent(out, indent_level)
        << "<UV> " << inline_name << "{ " << get_uvw() << " }\n";
    } else {
      indent(out, indent_level)
        << "<UV> " << inline_name << "{ " << get_uv() << " }\n";
    }
  } else {
    indent(out, indent_level) << "<UV> " << inline_name << "{\n";
    if (has_w()) {
      indent(out, indent_level+2) << get_uvw() << "\n";
    } else {
      indent(out, indent_level+2) << get_uv() << "\n";
    }
    if (has_tangent()) {
      indent(out, indent_level + 2)
        << "<Tangent> { " << get_tangent() << " }\n";
    }
    if (has_binormal()) {
      indent(out, indent_level + 2)
        << "<Binormal> { " << get_binormal() << " }\n";
    }
    _duvs.write(out, indent_level + 2, "<Duv>", get_num_dimensions());
    indent(out, indent_level) << "}\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexUV::compare_to
//       Access: Public
//  Description: An ordering operator to compare two vertices for
//               sorting order.  This imposes an arbitrary ordering
//               useful to identify unique vertices.
////////////////////////////////////////////////////////////////////
int EggVertexUV::
compare_to(const EggVertexUV &other) const {
  if (_flags != other._flags) {
    return _flags - other._flags;
  }
  int compare;
  compare = _uvw.compare_to(other._uvw, egg_parameters->_uv_threshold);
  if (compare != 0) {
    return compare;
  }

  if (has_tangent()) {
    compare = _tangent.compare_to(other._tangent, egg_parameters->_normal_threshold);
    if (compare != 0) {
      return compare;
    }
  }

  if (has_binormal()) {
    compare = _binormal.compare_to(other._binormal, egg_parameters->_normal_threshold);
    if (compare != 0) {
      return compare;
    }
  }

  if (_duvs != other._duvs) {
    return _duvs < other._duvs ? -1 : 1;
  }

  return 0;
}

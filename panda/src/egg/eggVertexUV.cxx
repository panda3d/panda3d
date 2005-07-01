// Filename: eggVertexUV.cxx
// Created by:  drose (20Jul04)
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
EggVertexUV(const string &name, const TexCoordd &uv) :
  EggNamedObject(name),
  _flags(0),
  _uv(uv)
{
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
  _uv(copy._uv)
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
  _uv = copy._uv;

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

  if (_duvs.empty() && _flags == 0) {
    indent(out, indent_level)
      << "<UV> " << inline_name << "{ " << get_uv() << " }\n";
  } else {
    indent(out, indent_level) << "<UV> " << inline_name << "{\n";
    indent(out, indent_level+2) << get_uv() << "\n";
    if (has_tangent()) {
      indent(out, indent_level + 2)
        << "<Tangent> { " << get_tangent() << " }\n";
    }
    if (has_binormal()) {
      indent(out, indent_level + 2)
        << "<Binormal> { " << get_binormal() << " }\n";
    }
    _duvs.write(out, indent_level+2);
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
  compare = _uv.compare_to(other._uv, egg_parameters->_uv_threshold);
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

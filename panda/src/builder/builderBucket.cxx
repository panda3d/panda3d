// Filename: builderBucket.cxx
// Created by:  drose (10Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "builderAttrib.h"
#include "builderBucket.h"
#include "builderFuncs.h"
#include "builderMisc.h"
#include "geomNode.h"


BuilderBucket *BuilderBucket::_default_bucket = NULL;


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BuilderBucket::
BuilderBucket() {
  _node = NULL;
  (*this) = (*get_default_bucket());
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BuilderBucket::
BuilderBucket(const BuilderBucket &copy) {
  (*this) = copy;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BuilderBucket &BuilderBucket::
operator = (const BuilderBucket &copy) {
  ((BuilderProperties &)*this) = (BuilderProperties &)copy;

  set_name(copy.get_name());
  set_coords(copy._coords);
  set_normals(copy._normals);
  set_texcoords(copy._texcoords);
  set_colors(copy._colors);

  _node = copy._node;
  _hidden = copy._hidden;
  _state = copy._state;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BuilderBucket::
~BuilderBucket() {
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of this object.  If
//               you are subclassing from BuilderBucket, you must
//               redefine this to return an instance of your new
//               subclass, because the Builder will call this function
//               to get its own copy.
////////////////////////////////////////////////////////////////////
BuilderBucket *BuilderBucket::
make_copy() const {
  return new BuilderBucket(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::make_geom_node
//       Access: Public, Virtual
//  Description: Called by the builder when it is time to create a new
//               GeomNode.  This function should allocate and return a
//               new GeomNode suitable for adding geometry to.  You
//               may redefine it to return a subclass of GeomNode, or
//               to do some initialization to the node.
////////////////////////////////////////////////////////////////////
GeomNode *BuilderBucket::
make_geom_node() {
  return new GeomNode("");
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::done_geom
//       Access: Public, Virtual
//  Description: Called after all the geometry has been added to the
//               Geom.  This is just a hook for the user to redefine
//               to do any post-processing that may be desired on the
//               geometry.  It may deallocate it and return a new
//               copy.  If it returns NULL, the geom is discarded.
////////////////////////////////////////////////////////////////////
Geom *BuilderBucket::
done_geom(Geom *geom) {
  return geom;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::add_attrib
//       Access: Public
//  Description: A convenience function to add the indicated render
//               attribute to the bucket's state.
////////////////////////////////////////////////////////////////////
void BuilderBucket::
add_attrib(const RenderAttrib *attrib) {
  _state = _state->add_attrib(attrib);
}



////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::Ordering operator
//       Access: Public, Virtual
//  Description: Defines an arbitrary ordering among different
//               buckets, and groups identical buckets together.
//               (Buckets a and b are identical if !(a < b) and !(b <
//               a).)
//
//               The actual order between different buckets is
//               arbitrary and largely irrelevant, so long as it is
//               consistent.  That is, if (a < b) and (b < c), it must
//               also be true that (a < c).  Also, if (a < b), it
//               cannot be true that (b < a).
////////////////////////////////////////////////////////////////////
bool BuilderBucket::
operator < (const BuilderBucket &other) const {
  if (get_name() != other.get_name()) {
    return get_name() < other.get_name();
  }

  if (_node != other._node) {
    return _node < other._node;
  }

  if (_hidden != other._hidden) {
    return _hidden < other._hidden;
  }

  if (_coords != other._coords)
    return _coords < other._coords;
  if (_normals != other._normals)
    return _normals < other._normals;
  if (_texcoords != other._texcoords)
    return _texcoords < other._texcoords;
  if (_colors != other._colors)
    return _colors < other._colors;

  if (_state != other._state) {
    return _state < other._state;
  }

  return BuilderProperties::operator < (other);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::output
//       Access: Public, Virtual
//  Description: Formats the bucket for output in some sensible way.
////////////////////////////////////////////////////////////////////
void BuilderBucket::
output(ostream &out) const {
  out << "Bucket \"" << get_name() << "\"";

  if (_node != (PandaNode *)NULL) {
    out << " attached to " << *_node << "\n";
  }
  out << "\n";

  if (_hidden) {
    out << "_hidden\n";
  }

  if (_coords != (Vertexf *)NULL) {
    out << "_coords = " << (void *)_coords << "\n";
  }

  if (_normals != (Normalf *)NULL) {
    out << "_normals = " << (void *)_normals << "\n";
  }

  if (_texcoords != (TexCoordf *)NULL) {
    out << "_texcoords = " << (void *)_texcoords << "\n";
  }

  if (_colors != (Colorf *)NULL) {
    out << "_colors = " << (void *)_colors << "\n";
  }

  if (!_state->is_empty()) {
    out << *_state << "\n";
  }

  BuilderProperties::output(out);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderBucket::private Constructor
//       Access: Private
//  Description: This special constructor is used only to initialize
//               the _default_bucket pointer.  It sets up the initial
//               defaults.  The normal constructor copies from this
//               instance.
////////////////////////////////////////////////////////////////////
BuilderBucket::
BuilderBucket(int) {
  _node = NULL;
  _hidden = false;

  // From BuilderProperties
  _mesh = true;
  _retesselate_coplanar = true;
  _show_tstrips = false;
  _show_qsheets = false;
  _show_quads = false;
  _show_normals = false;
  _normal_color.set(1.0, 0.0, 0.0, 1.0);
  _normal_scale = 1.0;
  _subdivide_polys = true;
  _coplanar_threshold = 0.01;

  _unroll_fans = true;
  _consider_fans = true;
  _max_tfan_angle = 40.0;
  _min_tfan_tris = 0;

  _state = RenderState::make_empty();
}

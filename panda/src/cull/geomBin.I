// Filename: geomBin.I
// Created by:  drose (13Apr00)
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

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomBin::
GeomBin(const string &name) :
  Namable(name)
{
  _traverser = (CullTraverser *)NULL;
  _is_attached = false;
  _sort = 0;
  _active = true;
  _parent = (GeomBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::get_sort
//       Access: Public
//  Description: Returns the sort index associated with this
//               particular bin.  The CullTraverser will render bins
//               in order according to their sort index.
////////////////////////////////////////////////////////////////////
INLINE int GeomBin::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::get_active
//       Access: Public
//  Description: Returns the active flag of this particular bin.  If
//               the flag is false, the contents of the bin are not
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE bool GeomBin::
is_active() const {
  return _active;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::has_traverser
//       Access: Public
//  Description: Returns true if the GeomBin is currently attached to
//               a traverser, false otherwise.  If this is false, and
//               the bin is not attached to a parent bin
//               (i.e. has_parent() is also false), don't expect the
//               bin to be rendered.
////////////////////////////////////////////////////////////////////
INLINE bool GeomBin::
has_traverser() const {
  return (_traverser != (CullTraverser *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::get_traverser
//       Access: Public
//  Description: Returns a pointer to the CullTraverser this bin is
//               currently attached to, or NULL if it is not attached.
////////////////////////////////////////////////////////////////////
INLINE CullTraverser *GeomBin::
get_traverser() const {
  return _traverser;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::has_parent
//       Access: Public
//  Description: Returns true if the bin is a child of some
//               GeomBinGroup, false if it is a toplevel bin in its
//               own right.  See GeomBinGroup.
////////////////////////////////////////////////////////////////////
INLINE bool GeomBin::
has_parent() const {
  return _parent != (GeomBin *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::get_parent
//       Access: Public
//  Description: Returns the parent pointer of the GeomBin if the bin
//               is a child of some GeomBinGropu, or NULL if the bin
//               does not have a parent.  See GeomBinGroup.
////////////////////////////////////////////////////////////////////
INLINE GeomBin *GeomBin::
get_parent() const {
  return _parent;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::is_attached
//       Access: Public
//  Description: Returns true if the bin is currently attached to a
//               CullTraverser, false otherwise.  This should always
//               be tree if has_traverser() is true, and false if
//               has_traverser() is false.
////////////////////////////////////////////////////////////////////
INLINE bool GeomBin::
is_attached() const {
  return _is_attached;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::claim_cull_state
//       Access: Protected
//  Description: Marks the indicated CullState as being assigned to
//               this bin, presumably in preparation to storing its
//               pointer within the bin.
////////////////////////////////////////////////////////////////////
INLINE void GeomBin::
claim_cull_state(CullState *cs) {
  nassertv(cs != (CullState *)NULL);

  if (cs->_bin != (GeomBin *)NULL && cs->_bin != this) {
    cs->_bin->remove_state(cs);
  }

  cs->_bin = this;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::claim_cull_state
//       Access: Protected
//  Description: Marks the indicated CullState as being assigned to
//               no particular bin, presumably in preparation to
//               removing it from the bin.
////////////////////////////////////////////////////////////////////
INLINE void GeomBin::
disclaim_cull_state(CullState *cs) {
  nassertv(cs != (CullState *)NULL);
  nassertv(cs->_bin == this);
  cs->_bin = (GeomBin *)NULL;
}

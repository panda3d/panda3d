// Filename: geomBin.I
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBin::
GeomBin(const string &name, CullTraverser *traverser, int sort) :
  Namable(name) 
{
  _traverser = (CullTraverser *)NULL;
  if (traverser != (CullTraverser *)NULL) {
    attach_to(traverser, sort);
  }
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
//     Function: GeomBin::set_sort
//       Access: Public
//  Description: Changes the sort index associated with this
//               particular bin.  The CullTraverser will render bins
//               in order according to their sort index.
////////////////////////////////////////////////////////////////////
INLINE void GeomBin::
set_sort(int sort) {
  if (_traverser != (CullTraverser *)NULL) {
    attach_to(_traverser, sort);
  } else {
    _sort = sort;
  }
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

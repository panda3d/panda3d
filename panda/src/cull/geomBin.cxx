// Filename: geomBin.cxx
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "geomBin.h"
#include "cullTraverser.h"
#include "config_cull.h"

#include <indent.h>
#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>

TypeHandle GeomBin::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomBin::
~GeomBin() {
  // We shouldn't be attached to anything when we destruct.  If we
  // are, something went screwy in the reference counting.
  nassertv(_traverser == (CullTraverser *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::attach_to
//       Access: Public
//  Description: Detaches the bin from whichever CullTraverser it is
//               currently attached to, and attaches it to the
//               indicated CullTraverser instead, at the indicated
//               sort level.  The CullTraverser will render all of its
//               attached GeomBins, in order according to sort level.
////////////////////////////////////////////////////////////////////
void GeomBin::
attach_to(CullTraverser *traverser, int sort) {
  nassertv(traverser != (CullTraverser *)NULL);
  PT(GeomBin) keep = detach();
  _sort = sort;
  _traverser = traverser;
  _traverser->_bins.insert(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::detach
//       Access: Public
//  Description: Detaches the bin from whichever CullTraverser it is
//               currently attached to, if any.  The bin will no
//               longer be rendered.  The return value is a
//               PT(GeomBin) that refers to the GeomBin itself; the
//               caller should save this pointer in a local PT
//               variable to avoid possibly destructing the GeomBin
//               (since detaching it may remove the last outstanding
//               reference count).
////////////////////////////////////////////////////////////////////
PT(GeomBin) GeomBin::
detach() {
  PT(GeomBin) keep = this;
  if (_traverser != (CullTraverser *)NULL) {
    _traverser->_bins.erase(this);
  }
  _traverser = (CullTraverser *)NULL;
  return keep;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::remove_state
//       Access: Public, Virtual
//  Description: Disassociates the indicated state from the bin, if it
//               was previously associated; presumably because a
//               change in the scene's initial attributes as resulted
//               in the indicated CullState switching to a new bin.
//               
//               Since the initial attributes will remain constant
//               throughout a given frame, this function will only be
//               called for GeomBins that save CullStates between
//               frames; simple GeomBins that empty the entire list of
//               CullStates upon a call to clear_current_states()
//               should never see this function.
////////////////////////////////////////////////////////////////////
void GeomBin::
remove_state(CullState *) {
  cull_cat.error()
    << "remove_state() unexpectedly called for " << get_type() << "\n";
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomBin::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomBin::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

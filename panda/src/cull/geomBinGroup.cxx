// Filename: geomBinGroup.cxx
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


#include "geomBinGroup.h"
#include "cullTraverser.h"
#include <indent.h>
#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>

TypeHandle GeomBinGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GeomBinGroup::
~GeomBinGroup() {
  nassertv(!is_attached());

  // Disassociate all of our children before we destruct.
  SubBins::iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    GeomBin *sub_bin = (*sbi);
    sub_bin->_parent = (GeomBin *)NULL;
  }
  _sub_bins.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::add_sub_bin
//       Access: Public
//  Description: Appends the indicated bin to the end of the sub_bin
//               list, and returns the new index.
////////////////////////////////////////////////////////////////////
int GeomBinGroup::
add_sub_bin(GeomBin *sub_bin) {
  nassertr(!sub_bin->has_parent(), -1);
  nassertr(!sub_bin->is_attached(), -1);
  nassertr(sub_bin->has_name(), -1);

  sub_bin->_parent = this;
  _sub_bins.push_back(sub_bin);

  if (is_attached()) {
    sub_bin->_traverser = _traverser;
    _traverser->attach_sub_bin(sub_bin);
  }

  return _sub_bins.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::remove_bin
//       Access: Public
//  Description: Removes the nth bin.  All subsequent index numbers
//               shift down by one.  The return value is the
//               just-removed bin, which may be deleted if it is not
//               immediately saved.
////////////////////////////////////////////////////////////////////
PT(GeomBin) GeomBinGroup::
remove_bin(int n) {
  nassertr(n >= 0 && n < (int)_sub_bins.size(), NULL);
  PT(GeomBin) sub_bin = get_bin(n);
  nassertr(sub_bin->get_parent() == this, NULL);

  if (is_attached()) {
    nassertr(sub_bin->is_attached(), NULL);
    nassertr(sub_bin->_traverser == _traverser, NULL);
    _traverser->detach_sub_bin(sub_bin);
    sub_bin->_traverser = (CullTraverser *)NULL;
  }

  _sub_bins.erase(_sub_bins.begin() + n);
  sub_bin->_parent = (GeomBin *)NULL;

  return sub_bin;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::set_active
//       Access: Public, Virtual
//  Description: Sets the active flag of this particular bin, and all
//               of its child bins.  If the flag is false, the
//               contents of the bin are not rendered.
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
set_active(bool active) {
  GeomBin::set_active(active);

  SubBins::iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->set_active(active);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::clear_current_states
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to reset the
//               list of CullStates that were added last frame, in
//               preparation for defining a new set of CullStates
//               visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
clear_current_states() {
  // This should never be called for toplevel bins, only sub bins--and
  // a GeomBinGroup should never be a sub bin.  The function will do
  // the right thing even if it is called, but this is probably a
  // mistake.
  cull_cat.warning()
    << "GeomBinGroup::clear_current_states() called\n";

  SubBins::iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->clear_current_states();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinUnsorted::record_current_state
//       Access: Public, Virtual
//  Description: Called each frame by the CullTraverser to indicated
//               that the given CullState (and all of its current
//               GeomNodes) is visible this frame.
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
record_current_state(GraphicsStateGuardian *gsg, CullState *cs,
                     int draw_order, CullTraverser *trav) {
  int index = choose_bin(cs);
  if (index >= 0 && index < get_num_bins()) {
    get_bin(index)->record_current_state(gsg, cs, draw_order, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::draw
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
draw(CullTraverser *trav) {
  // This should never be called for toplevel bins, only sub bins--and
  // a GeomBinGroup should never be a sub bin.  The function will do
  // the right thing even if it is called, but this is probably a
  // mistake.
  cull_cat.warning()
    << "GeomBinGroup::draw() called\n";

  SubBins::iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->draw(trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
output(ostream &out) const {
  out << get_type() << " " << get_name()
      << ", " << _sub_bins.size() << " bins.";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_name() << " {\n";

  SubBins::const_iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    (*sbi)->write(out, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::attach
//       Access: Protected, Virtual
//  Description: Formally adds the GeomBin to its indicated Traverser.
////////////////////////////////////////////////////////////////////
void GeomBinGroup::
attach() {
  nassertv(has_name());
  nassertv(_traverser != (CullTraverser *)NULL);
  nassertv(!_is_attached);

  // In the case of a GeomBinGroup, the bin is only a toplevel bin
  // (visible by name from the CullTraverser), and not a sub bin
  // (directly renderable).
  if (!has_parent()) {
    _traverser->attach_toplevel_bin(this);
  }

  _is_attached = true;

  // Now attach each of our sub bins.
  SubBins::const_iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    GeomBin *sub_bin = (*sbi);
    nassertv(!sub_bin->is_attached());
    sub_bin->_traverser = _traverser;
    sub_bin->attach();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinGroup::detach
//       Access: Protected, Virtual
//  Description: Detaches the bin from whichever CullTraverser it is
//               currently attached to.  The bin will no longer be
//               rendered.  The return value is a PT(GeomBin) that
//               refers to the GeomBin itself; the caller should save
//               this pointer in a local PT variable to avoid possibly
//               destructing the GeomBin (since detaching it may
//               remove the last outstanding reference count).
////////////////////////////////////////////////////////////////////
PT(GeomBin) GeomBinGroup::
detach() {
  nassertr(_is_attached, NULL);
  nassertr(_traverser != (CullTraverser *)NULL, NULL);

  PT(GeomBin) keep = this;

  if (!has_parent()) {
    _traverser->detach_toplevel_bin(this);
  }

  _is_attached = false;

  // Now detach each of our sub bins.
  SubBins::const_iterator sbi;
  for (sbi = _sub_bins.begin(); sbi != _sub_bins.end(); ++sbi) {
    GeomBin *sub_bin = (*sbi);
    sub_bin->detach();
    sub_bin->_traverser = (CullTraverser *)NULL;
  }

  return keep;
}

// Filename: geomBin.cxx
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "geomBin.h"
#include "cullTraverser.h"
#include "config_cull.h"
#include "geomBinNormal.h"
#include "geomBinUnsorted.h"
#include "geomBinFixed.h"
#include "geomBinBackToFront.h"

#include <indent.h>
#include <nodeAttributes.h>
#include <graphicsStateGuardian.h>
#include <string_utils.h>

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
  nassertv(!_is_attached);
  nassertv(_traverser == (CullTraverser *)NULL);

  // We also shouldn't be part of any parent bin.
  nassertv(_parent == (GeomBin *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::set_name
//       Access: Public
//  Description: Changes the name of the GeomBin, correctly updating
//               the CullTraverser it's attached to.
////////////////////////////////////////////////////////////////////
void GeomBin::
set_name(const string &name) {
  if (name == get_name()) {
    return;
  }

  // If we're currently attached, we need to need to be unattached
  // while we change the name--but only if we're a toplevel bin.  A
  // nested bin doesn't matter.
  if (is_attached() && !has_parent()) {
    PT(GeomBin) keep = this;

    nassertv(_traverser != (CullTraverser *)NULL);
    _traverser->detach_toplevel_bin(this);
    Namable::set_name(name);
    _traverser->attach_toplevel_bin(this);

  } else {
    Namable::set_name(name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::clear_name
//       Access: Public
//  Description: Removes the name of the GeomBin.  This also detaches
//               it (if it is a toplevel bin), and may therefore
//               inadvertently delete it!  Be very careful.
////////////////////////////////////////////////////////////////////
void GeomBin::
clear_name() {
  PT(GeomBin) keep;

  if (is_attached() && !has_parent()) {
    cull_cat.warning()
      << "GeomBin::clear_name() called on an attached GeomBin.\n";
    keep = detach();
  }

  Namable::clear_name();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::set_sort
//       Access: Public
//  Description: Changes the sort index associated with this
//               particular bin.  The CullTraverser will render bins
//               in order according to their sort index.
////////////////////////////////////////////////////////////////////
void GeomBin::
set_sort(int sort) {
  if (sort == _sort) {
    return;
  }

  // If we're currently attached, even if we're a nested bin, we need
  // to be unattached while we adjust the sorting order.
  if (is_attached()) {
    PT(GeomBin) keep = this;

    nassertv(_traverser != (CullTraverser *)NULL);
    _traverser->detach_sub_bin(this);
    _sort = sort;
    _traverser->attach_sub_bin(this);

  } else {
    _sort = sort;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::set_active
//       Access: Public, Virtual
//  Description: Sets the active flag of this particular bin.  If the
//               flag is false, the contents of the bin are not
//               rendered.
////////////////////////////////////////////////////////////////////
void GeomBin::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::set_traverser
//       Access: Public
//  Description: Associates the GeomBin with the indicated
//               CullTraverser.  A particular bin may only be
//               associated with one traverser at a time.  This is
//               normally done only after initially creating the
//               GeomBin; it is an error to call set_traverser() more
//               than once on the same bin (without calling
//               clear_traverser() first).
//
//               It is also not necessary (and is an error) to assign
//               child bins of a GeomBinGroup to a traverser; just
//               assign the parent bin.
//
//               This must be called before the GeomBin can be
//               rendered; it will thereafter be rendered by the
//               indicated traverser.
////////////////////////////////////////////////////////////////////
void GeomBin::
set_traverser(CullTraverser *traverser) {
  if (traverser == (CullTraverser *)NULL) {
    clear_traverser();
  }

  nassertv(!is_attached());
  nassertv(_traverser == (CullTraverser *)NULL);

  _traverser = traverser;
  attach();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::clear_traverser
//       Access: Public
//  Description: Disassociates the GeomBin with its current traverser.
//               The bin will no longer be rendered, and may even be
//               deleted if you do not immediately save the returned
//               pointer.
////////////////////////////////////////////////////////////////////
PT(GeomBin) GeomBin::
clear_traverser() {
  PT(GeomBin) keep = this;

  if (is_attached()) {
    detach();
    _traverser = (CullTraverser *)NULL;
  }
  
  return keep;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::parse_bin_type
//       Access: Public, Static
//  Description: Converts from the given string representation to one
//               of the derived GeomBin type handles.  Returns
//               TypeHandle::none() if the string does not match any
//               known bin type.
////////////////////////////////////////////////////////////////////
TypeHandle GeomBin::
parse_bin_type(const string &bin_type) {
  if (cmp_nocase_uh(bin_type, "normal") == 0) {
    return GeomBinNormal::get_class_type();

  } else if (cmp_nocase_uh(bin_type, "unsorted") == 0) {
    return GeomBinUnsorted::get_class_type();

  } else if (cmp_nocase_uh(bin_type, "state_sorted") == 0) {
    // For now, GeomBinUnsorted stands in surprisingly well for
    // GeomBinStateSorted.  This is because the states are already
    // reasonably sorted as they come out of the CullTraverser, so it
    // doesn't matter much whether the bin sorts it further.
    return GeomBinUnsorted::get_class_type();

  } else if (cmp_nocase_uh(bin_type, "statesorted") == 0) {
    return GeomBinUnsorted::get_class_type();

  } else if (cmp_nocase_uh(bin_type, "fixed") == 0) {
    return GeomBinFixed::get_class_type();

  } else if (cmp_nocase_uh(bin_type, "back_to_front") == 0) {
    return GeomBinBackToFront::get_class_type();

  } else if (cmp_nocase_uh(bin_type, "backtofront") == 0) {
    return GeomBinBackToFront::get_class_type();

  } else {
    return TypeHandle::none();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::make_bin
//       Access: Public, Static
//  Description: Creates and returns a new GeomBin of the appropriate
//               type as indicated by the TypeHandle.  Returns NULL if
//               the TypeHandle does not reflect a known GeomBin type.
////////////////////////////////////////////////////////////////////
PT(GeomBin) GeomBin::
make_bin(TypeHandle type, const string &name) {
  if (type == GeomBinNormal::get_class_type()) {
    return new GeomBinNormal(name);

  } else if (type == GeomBinUnsorted::get_class_type()) {
    return new GeomBinUnsorted(name);

  } else if (type == GeomBinFixed::get_class_type()) {
    return new GeomBinFixed(name);

  } else if (type == GeomBinBackToFront::get_class_type()) {
    return new GeomBinBackToFront(name);

  } else {
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::attach
//       Access: Protected, Virtual
//  Description: Formally adds the GeomBin to its indicated Traverser.
////////////////////////////////////////////////////////////////////
void GeomBin::
attach() {
  nassertv(has_name());
  nassertv(_traverser != (CullTraverser *)NULL);
  nassertv(!_is_attached);

  // In the ordinary case, a bin is both a toplevel bin (visible by
  // name from the CullTraverser) and a sub bin (directly renderable).
  if (!has_parent()) {
    _traverser->attach_toplevel_bin(this);
  }
  _traverser->attach_sub_bin(this);

  _is_attached = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBin::detach
//       Access: Protected, Virtual
//  Description: Detaches the bin from whichever CullTraverser it is
//               currently attached to.  The bin will no longer be
//               rendered.  The return value is a PT(GeomBin) that
//               refers to the GeomBin itself; the caller should save
//               this pointer in a local PT variable to avoid possibly
//               destructing the GeomBin (since detaching it may
//               remove the last outstanding reference count).
////////////////////////////////////////////////////////////////////
PT(GeomBin) GeomBin::
detach() {
  nassertr(_is_attached, NULL);
  nassertr(_traverser != (CullTraverser *)NULL, NULL);
    
  PT(GeomBin) keep = this;

  if (!has_parent()) {
    _traverser->detach_toplevel_bin(this);
  }
  _traverser->detach_sub_bin(this);

  _is_attached = false;

  return keep;
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

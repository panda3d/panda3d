// Filename: renderState.I
// Created by:  drose (21Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: RenderState::Composition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RenderState::Composition::
Composition() {
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Composition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RenderState::Composition::
Composition(const RenderState::Composition &copy) :
  _result(copy._result)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(const RenderAttrib *attrib, int override) :
  _type(attrib->get_type()),
  _attrib(attrib),
  _override(override)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Constructor
//       Access: Public
//  Description: This constructor is only used when reading the
//               RenderState from a bam file.  At this point, the
//               attribute pointer is unknown.
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(int override) :
  _override(override)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Constructor
//       Access: Public
//  Description: This constructor makes an invalid Attribute with no
//               RenderAttrib pointer; its purpose is just to make an
//               object we can use to look up a particular type in the
//               Attribute set.
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(TypeHandle type) :
  _type(type),
  _attrib(NULL),
  _override(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderState::Attribute::
Attribute(const Attribute &copy) :
  _type(copy._type),
  _attrib(copy._attrib),
  _override(copy._override)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void RenderState::Attribute::
operator = (const Attribute &copy) {
  _type = copy._type;
  _attrib = copy._attrib;
  _override = copy._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::operator <
//       Access: Public
//  Description: This is used by the Attributes set to uniquify
//               RenderAttributes by type.  Only one RenderAttrib of a
//               given type is allowed in the set.  This ordering must
//               also match the ordering reported by compare_to().
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::Attribute::
operator < (const Attribute &other) const {
  return _type < other._type;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::Attribute::compare_to
//       Access: Public
//  Description: Provides an indication of whether a particular
//               attribute is equivalent to another one, for purposes
//               of generating unique RenderStates.  This should
//               compare all properties of the Attribute, but it is
//               important that the type is compared first, to be
//               consistent with the ordering defined by operator <.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::Attribute::
compare_to(const Attribute &other) const {
  if (_type != other._type) {
    return _type.get_index() - other._type.get_index();
  }
  if (_attrib != other._attrib) {
    return _attrib < other._attrib ? -1 : 1;
  }
  return _override - other._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::is_empty
//       Access: Published
//  Description: Returns true if the state is empty, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
is_empty() const {
  return _attributes.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_num_attribs
//       Access: Published
//  Description: Returns the number of separate attributes indicated
//               in the state.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_num_attribs() const {
  return _attributes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_attrib
//       Access: Published
//  Description: Returns the nth attribute in the state.
////////////////////////////////////////////////////////////////////
INLINE const RenderAttrib *RenderState::
get_attrib(int n) const {
  nassertr(n >= 0 && n < (int)_attributes.size(), NULL);
  return _attributes[n]._attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_override
//       Access: Published
//  Description: Returns the override associated with the nth
//               attribute in the state.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_override(int n) const {
  nassertr(n >= 0 && n < (int)_attributes.size(), 0);
  return _attributes[n]._override;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_draw_order
//       Access: Published
//  Description: Returns the draw order indicated by the
//               CullBinAttrib, if any, associated by this state (or 0
//               if there is no CullBinAttrib).  See get_bin_index().
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_draw_order() const {
  if ((_flags & F_checked_bin_index) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal draw_order cache.
    ((RenderState *)this)->determine_bin_index();
  }
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_fog
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a FogAttrib on this state.  It returns a
//               pointer to the FogAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const FogAttrib *RenderState::
get_fog() const {
  if ((_flags & F_checked_fog) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal fog cache.
    ((RenderState *)this)->determine_fog();
  }
  return _fog;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_bin
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a BinAttrib on this state.  It returns a
//               pointer to the BinAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const CullBinAttrib *RenderState::
get_bin() const {
  if ((_flags & F_checked_bin) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal bin cache.
    ((RenderState *)this)->determine_bin();
  }
  return _bin;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_transparency
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a TransparencyAttrib on this state.  It returns a
//               pointer to the TransparencyAttrib, if there is one,
//               or NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const TransparencyAttrib *RenderState::
get_transparency() const {
  if ((_flags & F_checked_transparency) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal transparency cache.
    ((RenderState *)this)->determine_transparency();
  }
  return _transparency;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_bin_index
//       Access: Published
//  Description: Returns the bin index indicated by the CullBinAttrib,
//               if any, associated by this state (or the default bin
//               index if there is no CullBinAttrib).  This function
//               is provided as an optimization for determining this
//               at render time.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
get_bin_index() const {
  if ((_flags & F_checked_bin_index) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal bin_index cache.
    ((RenderState *)this)->determine_bin_index();
  }
  return _bin_index;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::set_destructing
//       Access: Private
//  Description: This function should only be called from the
//               destructor; it indicates that this RenderState
//               object is beginning destruction.  It is only used as
//               a sanity check, and is only meaningful when NDEBUG is
//               not defined.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
set_destructing() {
#ifndef NDEBUG
  _flags |= F_is_destructing;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::is_destructing
//       Access: Private
//  Description: Returns true if the RenderState object is
//               currently within its destructor
//               (i.e. set_destructing() has been called).  This is
//               only used as a sanity check, and is only meaningful
//               when NDEBUG is not defined.
////////////////////////////////////////////////////////////////////
INLINE bool RenderState::
is_destructing() const {
#ifndef NDEBUG
  return (_flags & F_is_destructing) != 0;
#else
  return false;
#endif
}

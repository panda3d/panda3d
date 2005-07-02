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
//     Function: RenderState::cache_ref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
cache_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::cache_ref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::cache_ref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::cache_unref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
cache_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::cache_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::cache_unref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::node_ref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
node_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::node_ref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::node_ref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::node_unref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int RenderState::
node_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::node_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::node_unref();
#endif  // DO_PSTATS
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
//     Function: RenderState::get_color
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a ColorAttrib on this state.  It returns a
//               pointer to the ColorAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const ColorAttrib *RenderState::
get_color() const {
  if ((_flags & F_checked_color) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal color cache.
    ((RenderState *)this)->determine_color();
  }
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_color_scale
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a ColorScaleAttrib on this state.  It returns a
//               pointer to the ColorScaleAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const ColorScaleAttrib *RenderState::
get_color_scale() const {
  if ((_flags & F_checked_color_scale) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal color_scale cache.
    ((RenderState *)this)->determine_color_scale();
  }
  return _color_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_texture
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a TextureAttrib on this state.  It returns a
//               pointer to the TextureAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const TextureAttrib *RenderState::
get_texture() const {
  if ((_flags & F_checked_texture) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal texture cache.
    ((RenderState *)this)->determine_texture();
  }
  return _texture;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_tex_gen
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a TexGenAttrib on this state.  It returns a
//               pointer to the TexGenAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const TexGenAttrib *RenderState::
get_tex_gen() const {
  if ((_flags & F_checked_tex_gen) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal tex_gen cache.
    ((RenderState *)this)->determine_tex_gen();
  }
  return _tex_gen;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_tex_matrix
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a TexMatrixAttrib on this state.  It returns a
//               pointer to the TexMatrixAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const TexMatrixAttrib *RenderState::
get_tex_matrix() const {
  if ((_flags & F_checked_tex_matrix) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal tex_matrix cache.
    ((RenderState *)this)->determine_tex_matrix();
  }
  return _tex_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderState::get_render_mode
//       Access: Published
//  Description: This function is provided as an optimization, to
//               speed up the render-time checking for the existance
//               of a RenderModeAttrib on this state.  It returns a
//               pointer to the RenderModeAttrib, if there is one, or
//               NULL if there is not.
////////////////////////////////////////////////////////////////////
INLINE const RenderModeAttrib *RenderState::
get_render_mode() const {
  if ((_flags & F_checked_render_mode) == 0) {
    // We pretend this function is const, even though it transparently
    // modifies the internal render_mode cache.
    ((RenderState *)this)->determine_render_mode();
  }
  return _render_mode;
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

////////////////////////////////////////////////////////////////////
//     Function: RenderState::consider_update_pstats
//       Access: Private
//  Description: Calls update_pstats() if the state of the referenced
//               bits has changed from the indicated value.
////////////////////////////////////////////////////////////////////
INLINE void RenderState::
consider_update_pstats(int old_referenced_bits) const {
  int new_referenced_bits = get_referenced_bits();
  if (old_referenced_bits != new_referenced_bits) {
    update_pstats(old_referenced_bits, new_referenced_bits);
  }
}

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
//     Function: RenderState::CompositionCycleDescEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RenderState::CompositionCycleDescEntry::
CompositionCycleDescEntry(const RenderState *obj,
                          const RenderState *result,
                          bool inverted) :
  _obj(obj),
  _result(result),
  _inverted(inverted)
{
}

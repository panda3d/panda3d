// Filename: textureAttrib.I
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
//     Function: TextureAttrib::Constructor
//       Access: Protected
//  Description: Use TextureAttrib::make() to construct a new
//               TextureAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TextureAttrib::
TextureAttrib() {
  _off_all_stages = false;
  _sort_seq = UpdateSeq::old();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::Copy Constructor
//       Access: Protected
//  Description: Use TextureAttrib::make() to construct a new
//               TextureAttrib object.  The copy constructor is only
//               defined to facilitate methods like add_on_stage().
////////////////////////////////////////////////////////////////////
INLINE TextureAttrib::
TextureAttrib(const TextureAttrib &copy) :
  _on_stages(copy._on_stages),
  _off_stages(copy._off_stages),
  _off_all_stages(copy._off_all_stages),
  _on_textures(copy._on_textures),
  _sort_seq(copy._sort_seq)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::is_off
//       Access: Published
//  Description: Returns true if the TextureAttrib is an 'off'
//               TextureAttrib, indicating that it should disable
//               texturing.
//
//               This method is deprecated, and is provided for
//               backward compatibility; you should use the
//               multitexture form of this instead.
////////////////////////////////////////////////////////////////////
INLINE bool TextureAttrib::
is_off() const {
  return (_on_stages.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_texture
//       Access: Published
//  Description: If the TextureAttrib is not an 'off' TextureAttrib,
//               returns the base-level texture that is associated.
//               Otherwise, return NULL.
////////////////////////////////////////////////////////////////////
INLINE Texture *TextureAttrib::
get_texture() const {
  if (_on_stages.empty()) {
    return NULL;
  }
  check_sorted();
  return get_on_texture(filter_to_max(1)->get_on_stage(0));
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_num_on_stages
//       Access: Published
//  Description: Returns the number of stages that are turned on by
//               the attribute.
////////////////////////////////////////////////////////////////////
INLINE int TextureAttrib::
get_num_on_stages() const {
  check_sorted();
  return _on_stages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_on_stage
//       Access: Published
//  Description: Returns the nth stage turned on by the attribute,
//               sorted in render order.
////////////////////////////////////////////////////////////////////
INLINE TextureStage *TextureAttrib::
get_on_stage(int n) const {
  nassertr(n >= 0 && n < (int)_on_stages.size(), (TextureStage *)NULL);
  return _on_stages[n];
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::has_on_stage
//       Access: Published
//  Description: Returns true if the indicated stage is turned on by
//               the attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextureAttrib::
has_on_stage(TextureStage *stage) const {
  return _on_textures.find(stage) != _on_textures.end();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_on_texture
//       Access: Published
//  Description: Returns the texture associated with the indicated
//               stage, or NULL if no texture is associated.
////////////////////////////////////////////////////////////////////
INLINE Texture *TextureAttrib::
get_on_texture(TextureStage *stage) const {
  OnTextures::const_iterator ti;
  ti = _on_textures.find(stage);
  if (ti != _on_textures.end()) {
    return (*ti).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_num_off_stages
//       Access: Published
//  Description: Returns the number of stages that are turned off by
//               the attribute.
////////////////////////////////////////////////////////////////////
INLINE int TextureAttrib::
get_num_off_stages() const {
  return _off_stages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_off_stage
//       Access: Published
//  Description: Returns the nth stage turned off by the attribute,
//               sorted in arbitrary (pointer) order.
////////////////////////////////////////////////////////////////////
INLINE TextureStage *TextureAttrib::
get_off_stage(int n) const {
  nassertr(n >= 0 && n < (int)_off_stages.size(), (TextureStage *)NULL);
  return _off_stages[n];
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::has_off_stage
//       Access: Published
//  Description: Returns true if the indicated stage is turned off by
//               the attrib, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TextureAttrib::
has_off_stage(TextureStage *stage) const {
  return _off_stages.find(stage) != _off_stages.end() ||
    (_off_all_stages && !has_on_stage(stage));
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::has_all_off
//       Access: Published
//  Description: Returns true if this attrib turns off all stages
//               (although it may also turn some on).
////////////////////////////////////////////////////////////////////
INLINE bool TextureAttrib::
has_all_off() const {
  return _off_all_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::is_identity
//       Access: Published
//  Description: Returns true if this is an identity attrib: it does
//               not change the set of stages in use.
////////////////////////////////////////////////////////////////////
INLINE bool TextureAttrib::
is_identity() const {
  return _on_stages.empty() && _off_stages.empty() && !_off_all_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::get_on_stages
//       Access: Public
//  Description: Returns the complete list of stages defined within
//               this TextureAttrib, suitable for passing to
//               Geom::setup_multitexcoord_iterator().
////////////////////////////////////////////////////////////////////
INLINE const Geom::ActiveTextureStages &TextureAttrib::
get_on_stages() const {
  return _on_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttrib::check_sorted
//       Access: Private
//  Description: Confirms whether the _on_stages list is still sorted.
//               It will become unsorted if someone calls
//               TextureStage::set_sort().
//
//               If the list requires sorting, transparently sorts it
//               before returning.
////////////////////////////////////////////////////////////////////
INLINE void TextureAttrib::
check_sorted() const {
  if (_sort_seq != TextureStage::get_sort_seq()) {
    ((TextureAttrib *)this)->sort_on_stages();
  }
}

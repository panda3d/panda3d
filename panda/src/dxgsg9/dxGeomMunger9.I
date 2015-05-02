// Filename: dxGeomMunger9.I
// Created by:  drose (11Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DXGeomMunger9::
DXGeomMunger9(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 1, NT_packed_dabc, C_color),
  _texture(DCAST(TextureAttrib, state->get_attrib(TextureAttrib::get_class_slot()))),
  _tex_gen(DCAST(TexGenAttrib, state->get_attrib(TexGenAttrib::get_class_slot())))
{
  _filtered_texture = (TextureAttrib *)NULL;
  _reffed_filtered_texture = false;
  if (_texture != (TextureAttrib *)NULL) {
    _filtered_texture = _texture->filter_to_max(gsg->get_max_texture_stages());
    if (_filtered_texture != _texture) {
      _filtered_texture->ref();
      _reffed_filtered_texture = true;
    }
  }
  // Set a callback to unregister ourselves when either the Texture or
  // the TexGen object gets deleted.
  _texture.set_callback(this);
  _tex_gen.set_callback(this);
}


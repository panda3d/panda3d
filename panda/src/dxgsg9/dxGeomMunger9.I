/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGeomMunger9.I
 * @author drose
 * @date 2005-03-11
 */

/**
 *
 */
INLINE DXGeomMunger9::
DXGeomMunger9(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 1, NT_packed_dabc, C_color),
  _texture(nullptr),
  _tex_gen(nullptr)
{
  const TextureAttrib *texture = nullptr;
  const TexGenAttrib *tex_gen = nullptr;
  state->get_attrib(texture);
  state->get_attrib(tex_gen);
  _texture = texture;
  _tex_gen = tex_gen;

  _filtered_texture = nullptr;
  _reffed_filtered_texture = false;
  if (texture != nullptr) {
    _filtered_texture = texture->filter_to_max(gsg->get_max_texture_stages());
    if (_filtered_texture != texture) {
      _filtered_texture->ref();
      _reffed_filtered_texture = true;
    }
  }
  // Set a callback to unregister ourselves when either the Texture or the
  // TexGen object gets deleted.
  _texture.add_callback(this);
  _tex_gen.add_callback(this);
}

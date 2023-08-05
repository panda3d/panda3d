/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureContext.I
 * @author drose
 * @date 1999-10-07
 */

/**
 *
 */
INLINE TextureContext::
TextureContext(PreparedGraphicsObjects *pgo, Texture *tex) :
  BufferContext(&pgo->_texture_residency, tex),
  AdaptiveLruPage(0)
{
}

/**
 * Returns the pointer to the associated Texture object.
 */
INLINE Texture *TextureContext::
get_texture() const {
  return (Texture *)_object;
}

/**
 * @deprecated since 1.11.0: always returns 0.
 */
INLINE int TextureContext::
get_view() const {
  return 0;
}

/**
 * Returns true if the texture properties or image have been modified since
 * the last time mark_loaded() was called.
 */
INLINE bool TextureContext::
was_modified() const {
  return was_properties_modified() || was_image_modified();
}

/**
 * Returns true if the texture properties (unrelated to the image) have been
 * modified since the last time mark_loaded() was called.
 */
INLINE bool TextureContext::
was_properties_modified() const {
  return _properties_modified != get_texture()->get_properties_modified();
}

/**
 * Returns true if the texture image has been modified since the last time
 * mark_loaded() was called.
 */
INLINE bool TextureContext::
was_image_modified() const {
  return _image_modified != get_texture()->get_image_modified();
}

/**
 * Returns true if the given page of the texture image has been modified since
 * the last time mark_loaded() was called.
 */
INLINE bool TextureContext::
was_image_page_modified(int z, int n) const {
  return get_texture()->get_image_modified_pages(_image_modified, n).get_bit(z);
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the texture properties (unrelated to the image) are modified.
 */
INLINE UpdateSeq TextureContext::
get_properties_modified() const {
  return _properties_modified;
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * the texture image data (including mipmap levels) are modified.
 */
INLINE UpdateSeq TextureContext::
get_image_modified() const {
  return _image_modified;
}

/**
 * Returns a SparseArray indicating which pages of the texture have been
 * modified since the last call to mark_loaded().
 */
INLINE SparseArray TextureContext::
get_image_modified_pages(int n) const {
  return get_texture()->get_image_modified_pages(_image_modified, n);
}

/**
 * Returns a SparseArray indicating which pages of the texture have been
 * modified since the last call to mark_loaded().
 */
INLINE SparseArray TextureContext::
get_view_modified_pages(int view, int n) const {
  return get_texture()->get_view_modified_pages(_image_modified, view, n);
}

/**
 * Should be called (usually by a derived class) when the on-card size of this
 * object has changed.
 */
INLINE void TextureContext::
update_data_size_bytes(size_t new_data_size_bytes) {
  BufferContext::update_data_size_bytes(new_data_size_bytes);
  AdaptiveLruPage::set_lru_size(new_data_size_bytes);
}

/**
 * Should be called after the texture has been loaded into graphics memory,
 * this updates the internal flags for changed_size() and modified().
 */
INLINE void TextureContext::
mark_loaded() {
  // _data_size_bytes = _data->get_texture_size_bytes();
  _properties_modified = get_texture()->get_properties_modified();
  _image_modified = get_texture()->get_image_modified();
  update_modified((std::max)(_properties_modified, _image_modified));

  // Assume the texture is now resident.
  set_resident(true);
}

/**
 * Should be called after the texture has been forced out of texture memory.
 */
INLINE void TextureContext::
mark_unloaded() {
  _properties_modified = UpdateSeq::old();
  _image_modified = UpdateSeq::old();
  update_modified(UpdateSeq::old());

  set_resident(false);
}

/**
 * Should be called to indicate the texture should be reloaded at the nearest
 * opportunity.
 */
INLINE void TextureContext::
mark_needs_reload() {
  _image_modified = UpdateSeq::old();
}

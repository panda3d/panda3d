/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramTexture.I
 * @author rdb
 * @date 2014-12-11
 */

/**
 * Creates a new ParamTextureSampler storing the given texture and sampler
 * objects.
 */
INLINE ParamTextureSampler::
ParamTextureSampler(Texture *tex, const SamplerState &sampler) :
  _texture(tex),
  _sampler(sampler)
{
}

/**
 * Returns Texture::get_class_type(), even though it technically stores more
 * than just a Texture.
 */
INLINE TypeHandle ParamTextureSampler::
get_value_type() const {
  return Texture::get_class_type();
}

/**
 * Retrieves the texture stored in the parameter.
 */
INLINE Texture *ParamTextureSampler::
get_texture() const {
  return _texture;
}

/**
 * Retrieves the sampler state stored in the parameter.
 */
INLINE const SamplerState &ParamTextureSampler::
get_sampler() const {
  return _sampler;
}

/**
 * Creates a new ParamTextureImage storing the given texture and image binding
 * parameters.
 */
INLINE ParamTextureImage::
ParamTextureImage(Texture *tex, bool read, bool write, int z, int n) :
  _texture(tex),
  _access(0),
  _bind_level(std::min(n, 127)),
  _bind_layer(z)
{
  if (read) {
    _access |= A_read;
  }
  if (write) {
    _access |= A_write;
  }
  if (z < 0) {
    _bind_layer = 0;
    _access |= A_layered;
  }
}

/**
 * Returns Texture::get_class_type(), even though it technically stores more
 * than just a Texture.
 */
INLINE TypeHandle ParamTextureImage::
get_value_type() const {
  return Texture::get_class_type();
}

/**
 * Retrieves the texture stored in the parameter.
 */
INLINE Texture *ParamTextureImage::
get_texture() const {
  return _texture;
}

/**
 * Returns true if this image should be bound with read access enabled.
 */
INLINE bool ParamTextureImage::
has_read_access() const {
  return (_access & A_read) != 0;
}

/**
 * Returns true if this image should be bound with write access enabled.
 */
INLINE bool ParamTextureImage::
has_write_access() const {
  return (_access & A_write) != 0;
}

/**
 * Returns true if all layers of this image should be bound simultaneously.
 */
INLINE bool ParamTextureImage::
get_bind_layered() const {
  return (_access & A_layered) != 0;
}

/**
 * Returns the image level that should be bound.
 */
INLINE int ParamTextureImage::
get_bind_level() const {
  return _bind_level;
}

/**
 * Returns the image layer that should be bound.  This is undefined if
 * get_bind_layered() returns false.
 */
INLINE int ParamTextureImage::
get_bind_layer() const {
  return _bind_layer;
}

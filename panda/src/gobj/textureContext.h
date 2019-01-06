/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureContext.h
 * @author drose
 * @date 1999-10-07
 */

#ifndef TEXTURECONTEXT_H
#define TEXTURECONTEXT_H

#include "pandabase.h"

#include "bufferContext.h"
#include "texture.h"
#include "preparedGraphicsObjects.h"
#include "adaptiveLru.h"

/**
 * This is a special class object that holds all the information returned by a
 * particular GSG to indicate the texture's internal context identifier.
 *
 * Textures typically have an immediate-mode and a retained-mode operation.
 * When using textures in retained-mode (in response to Texture::prepare()),
 * the GSG will create some internal handle for the texture and store it here.
 * The texture stores all of these handles internally.
 */
class EXPCL_PANDA_GOBJ TextureContext : public BufferContext, public AdaptiveLruPage {
public:
  INLINE TextureContext(PreparedGraphicsObjects *pgo, Texture *tex, int view);

PUBLISHED:
  INLINE Texture *get_texture() const;
  INLINE int get_view() const;
  virtual uint64_t get_native_id() const;
  virtual uint64_t get_native_buffer_id() const;

  INLINE bool was_modified() const;
  INLINE bool was_properties_modified() const;
  INLINE bool was_image_modified() const;
  INLINE bool was_simple_image_modified() const;

  INLINE UpdateSeq get_properties_modified() const;
  INLINE UpdateSeq get_image_modified() const;
  INLINE UpdateSeq get_simple_image_modified() const;

public:
  INLINE void update_data_size_bytes(size_t new_data_size_bytes);
  INLINE void mark_loaded();
  INLINE void mark_simple_loaded();
  INLINE void mark_unloaded();
  INLINE void mark_needs_reload();

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

private:
  int _view;
  UpdateSeq _properties_modified;
  UpdateSeq _image_modified;
  UpdateSeq _simple_image_modified;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BufferContext::init_type();
    register_type(_type_handle, "TextureContext",
                  BufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

inline std::ostream &operator << (std::ostream &out, const TextureContext &context) {
  context.output(out);
  return out;
}

#include "textureContext.I"

#endif

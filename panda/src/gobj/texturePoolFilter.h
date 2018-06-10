/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePoolFilter.h
 * @author drose
 * @date 2006-07-27
 */

#ifndef TEXTUREPOOLFILTER_H
#define TEXTUREPOOLFILTER_H

#include "pandabase.h"
#include "texture.h"
#include "pointerTo.h"
#include "typedObject.h"

class LoaderOptions;

/**
 * This is an abstract base class, a placeholder for any number of different
 * classes that may wish to implement an effect on every texture loaded from
 * disk via the TexturePool.
 *
 * In practice, as of the time of this writing, only the TxaFileFilter (in
 * pandatool) actually implements this.  But other kinds of filters are
 * possible.
 *
 * This filter, once registered, will get a callback and a chance to modify
 * each texture as it is loaded from disk the first time.  If more than one
 * filter is registered, each will be called in sequence, in the order in
 * which they were registered.
 *
 * The filter does not get called again if the texture is subsequently
 * reloaded from disk.  It is suggested that filters for which this might be a
 * problem should call tex->set_keep_ram_image(true).
 */
class EXPCL_PANDA_GOBJ TexturePoolFilter : public TypedObject {
public:
  virtual ~TexturePoolFilter();

  virtual PT(Texture) pre_load(const Filename &orig_filename,
                               const Filename &orig_alpha_filename,
                               int primary_file_num_channels,
                               int alpha_file_channel,
                               bool read_mipmaps,
                               const LoaderOptions &options);
  virtual PT(Texture) post_load(Texture *tex);

  virtual void output(std::ostream &out) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "TexturePoolFilter",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const TexturePoolFilter &filter) {
  filter.output(out);
  return out;
}

#include "texturePoolFilter.I"

#endif

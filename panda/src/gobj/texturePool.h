/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePool.h
 * @author drose
 * @date 2000-04-26
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 */

#ifndef TEXTUREPOOL_H
#define TEXTUREPOOL_H

#include "pandabase.h"
#include "texture.h"
#include "filename.h"
#include "config_gobj.h"
#include "loaderOptions.h"
#include "pmutex.h"
#include "pmap.h"
#include "textureCollection.h"

class TexturePoolFilter;
class BamCache;
class BamCacheRecord;

/**
 * This is the preferred interface for loading textures from image files.  It
 * unifies all references to the same filename, so that multiple models that
 * reference the same textures don't waste texture memory unnecessarily.
 */
class EXPCL_PANDA_GOBJ TexturePool {
PUBLISHED:
  INLINE static bool has_texture(const Filename &filename);
  INLINE static bool verify_texture(const Filename &filename);
  BLOCKING INLINE static Texture *load_texture(const Filename &filename,
                                               int primary_file_num_channels = 0,
                                               bool read_mipmaps = false,
                                               const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE static Texture *load_texture(const Filename &filename,
                                               const Filename &alpha_filename,
                                               int primary_file_num_channels = 0,
                                               int alpha_file_channel = 0,
                                               bool read_mipmaps = false,
                                               const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE static Texture *load_3d_texture(const Filename &filename_pattern,
                                                  bool read_mipmaps = false,
                                                  const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE static Texture *load_2d_texture_array(const Filename &filename_pattern,
                                                        bool read_mipmaps = false,
                                                        const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE static Texture *load_cube_map(const Filename &filename_pattern,
                                                bool read_mipmaps = false,
                                                const LoaderOptions &options = LoaderOptions());

  INLINE static Texture *get_normalization_cube_map(int size);
  INLINE static Texture *get_alpha_scale_map();

  INLINE static void add_texture(Texture *texture);
  INLINE static void release_texture(Texture *texture);
  INLINE static void release_all_textures();
  INLINE static void rehash();

  INLINE static int garbage_collect();

  INLINE static void list_contents(std::ostream &out);
  INLINE static void list_contents();

  INLINE static Texture *find_texture(const std::string &name);
  INLINE static TextureCollection find_all_textures(const std::string &name = "*");

  INLINE static void set_fake_texture_image(const Filename &filename);
  INLINE static void clear_fake_texture_image();
  INLINE static bool has_fake_texture_image();
  INLINE static const Filename &get_fake_texture_image();
  INLINE static PT(Texture) make_texture(const std::string &extension);

  static void write(std::ostream &out);

public:
  typedef Texture::MakeTextureFunc MakeTextureFunc;
  void register_texture_type(MakeTextureFunc *func, const std::string &extensions);
  void register_filter(TexturePoolFilter *filter);

  MakeTextureFunc *get_texture_type(const std::string &extension) const;
  void write_texture_types(std::ostream &out, int indent_level) const;

  static TexturePool *get_global_ptr();

private:
  TexturePool();

  bool ns_has_texture(const Filename &orig_filename);
  Texture *ns_load_texture(const Filename &orig_filename,
                           int primary_file_num_channels,
                           bool read_mipmaps,
                           const LoaderOptions &options);
  Texture *ns_load_texture(const Filename &orig_filename,
                           const Filename &orig_alpha_filename,
                           int primary_file_num_channels,
                           int alpha_file_channel,
                           bool read_mipmaps,
                           const LoaderOptions &options);
  Texture *ns_load_3d_texture(const Filename &filename_pattern,
                              bool read_mipmaps,
                              const LoaderOptions &options);
  Texture *ns_load_2d_texture_array(const Filename &filename_pattern,
                                    bool read_mipmaps,
                                    const LoaderOptions &options);
  Texture *ns_load_cube_map(const Filename &filename_pattern,
                            bool read_mipmaps,
                            const LoaderOptions &options);
  Texture *ns_get_normalization_cube_map(int size);
  Texture *ns_get_alpha_scale_map();

  void ns_add_texture(Texture *texture);
  void ns_release_texture(Texture *texture);
  void ns_release_all_textures();
  int ns_garbage_collect();
  void ns_list_contents(std::ostream &out) const;
  Texture *ns_find_texture(const std::string &name) const;
  TextureCollection ns_find_all_textures(const std::string &name) const;
  PT(Texture) ns_make_texture(const std::string &extension) const;

  void resolve_filename(Filename &new_filename, const Filename &orig_filename,
                        bool read_mipmaps, const LoaderOptions &options);

  void try_load_cache(PT(Texture) &tex, BamCache *cache,
                      const Filename &filename, PT(BamCacheRecord) &record,
                      bool &compressed_cache_record,
                      const LoaderOptions &options);
  void report_texture_unreadable(const Filename &filename) const;

  // Methods to invoke a TexturePoolFilter.
  PT(Texture) pre_load(const Filename &orig_filename,
                       const Filename &orig_alpha_filename,
                       int primary_file_num_channels,
                       int alpha_file_channel,
                       bool read_mipmaps, const LoaderOptions &options);
  PT(Texture) post_load(Texture *tex);

  void load_filters();

  static TexturePool *_global_ptr;

  Mutex _lock;
  struct LookupKey {
    Filename _fullpath;
    Filename _alpha_fullpath;
    int _primary_file_num_channels = 0;
    int _alpha_file_channel = 0;
    Texture::TextureType _texture_type = Texture::TT_2d_texture;

    INLINE bool operator < (const LookupKey &other) const;
  };
  typedef pmap<LookupKey, PT(Texture)> Textures;
  Textures _textures;
  typedef pmap<Filename, Filename> RelpathLookup;
  RelpathLookup _relpath_lookup;

  Filename _fake_texture_image;

  PT(Texture) _normalization_cube_map;
  PT(Texture) _alpha_scale_map;

  typedef pmap<std::string, MakeTextureFunc *> TypeRegistry;
  TypeRegistry _type_registry;

  typedef pvector<TexturePoolFilter *> FilterRegistry;
  FilterRegistry _filter_registry;
};

#include "texturePool.I"

#endif

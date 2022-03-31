/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureImage.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef TEXTUREIMAGE_H
#define TEXTUREIMAGE_H

#include "pandatoolbase.h"

#include "imageFile.h"
#include "paletteGroups.h"
#include "textureRequest.h"

#include "namable.h"
#include "filename.h"
#include "pnmImage.h"
#include "eggRenderMode.h"

#include "pmap.h"
#include "pset.h"

class SourceTextureImage;
class DestTextureImage;
class TexturePlacement;
class EggFile;

/**
 * This represents a single source texture that is referenced by one or more
 * egg files.  It may be assigned to multiple PaletteGroups, and thus placed
 * on multiple PaletteImages (up to one per PaletteGroup).
 *
 * Since a TextureImage may be referenced by multiple egg files that are each
 * assigned to a different set of groups, it tries to maximize sharing between
 * egg files and minimize the number of different PaletteGroups it is assigned
 * to.
 */
class TextureImage : public ImageFile, public Namable {
public:
  TextureImage();

  void note_egg_file(EggFile *egg_file);
  void assign_groups();

  const PaletteGroups &get_groups() const;
  TexturePlacement *get_placement(PaletteGroup *group) const;
  void force_replace();
  void mark_eggs_stale();

  void mark_texture_named();
  bool is_texture_named() const;

  void pre_txa_file();
  void post_txa_file();
  bool got_txa_file() const;
  void determine_placement_size();

  bool get_omit() const;
  double get_coverage_threshold() const;
  int get_margin() const;
  bool is_surprise() const;
  bool is_used() const;
  EggRenderMode::AlphaMode get_alpha_mode() const;

  EggTexture::WrapMode get_txa_wrap_u() const;
  EggTexture::WrapMode get_txa_wrap_v() const;

  SourceTextureImage *get_source(const Filename &filename,
                                 const Filename &alpha_filename,
                                 int alpha_file_channel);

  SourceTextureImage *get_preferred_source();
  void clear_source_basic_properties();

  void copy_unplaced(bool redo_all);

  const PNMImage &read_source_image();
  void release_source_image();
  void set_source_image(const PNMImage &image);
  void read_header();
  bool is_newer_than(const Filename &reference_filename);

  void write_source_pathnames(std::ostream &out, int indent_level = 0) const;
  void write_scale_info(std::ostream &out, int indent_level = 0);

private:
  typedef pset<EggFile *> EggFiles;
  typedef pvector<EggFile *> WorkingEggs;
  typedef pmap<std::string, SourceTextureImage *> Sources;
  typedef pmap<std::string, DestTextureImage *> Dests;

  static int compute_egg_count(PaletteGroup *group,
                               const WorkingEggs &egg_files);

  void assign_to_groups(const PaletteGroups &groups);
  void consider_grayscale();
  void consider_alpha();

  void remove_old_dests(const Dests &a, const Dests &b);
  void copy_new_dests(const Dests &a, const Dests &b);

  std::string get_source_key(const Filename &filename,
                        const Filename &alpha_filename,
                        int alpha_file_channel);

private:
  TextureRequest _request;
  TextureProperties _pre_txa_properties;
  EggRenderMode::AlphaMode _pre_txa_alpha_mode;
  SourceTextureImage *_preferred_source;
  bool _is_surprise;

  bool _ever_read_image;
  bool _forced_grayscale;

  enum AlphaBits {
    // consider_alpha() sets alpha_bits to the union of all of these pixel
    // values that might be found in the alpha channel.
    AB_one   = 0x01,
    AB_mid   = 0x02,
    AB_zero  = 0x04,
    AB_all   = 0x07 // == AB_zero | AB_mid | AB_one
  };
  int _alpha_bits;
  double _mid_pixel_ratio;
  bool _is_cutout;
  EggRenderMode::AlphaMode _alpha_mode;
  EggTexture::WrapMode _txa_wrap_u, _txa_wrap_v;

  PaletteGroups _explicitly_assigned_groups;
  PaletteGroups _actual_assigned_groups;

  EggFiles _egg_files;

  typedef pmap<PaletteGroup *, TexturePlacement *> Placement;
  Placement _placement;

  Sources _sources;
  Dests _dests;

  bool _read_source_image;
  bool _allow_release_source_image;
  PNMImage _source_image;
  bool _texture_named;
  bool _got_txa_file;


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_TextureImage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // These values are only filled in while reading from the bam file; don't
  // use them otherwise.
  int _num_placement;
  int _num_sources;
  int _num_dests;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageFile::init_type();
    Namable::init_type();
    register_type(_type_handle, "TextureImage",
                  ImageFile::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class TxaLine;
};

#endif

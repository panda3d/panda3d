/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file palettizer.h
 * @author drose
 * @date 2000-12-01
 */

#ifndef PALETTIZER_H
#define PALETTIZER_H

#include "pandatoolbase.h"

#include "txaFile.h"

#include "typedWritable.h"
#include "eggRenderMode.h"
#include "pvector.h"
#include "pset.h"
#include "pmap.h"

class PNMFileType;
class EggFile;
class PaletteGroup;
class TextureImage;
class TexturePlacement;
class FactoryParams;

/**
 * This is the main engine behind egg-palettize.  It contains all of the
 * program parameters, from the command line or saved from a previous session,
 * and serves as the driving force in the actual palettizing process.
 */
class Palettizer : public TypedWritable {
public:
  Palettizer();

  bool get_noabs() const;
  void set_noabs(bool noabs);

  bool is_valid() const;
  void report_pi() const;
  void report_statistics() const;

  void read_txa_file(std::istream &txa_file, const std::string &txa_filename);
  void all_params_set();
  void process_command_line_eggs(bool force_texture_read, const Filename &state_filename);
  void process_all(bool force_texture_read, const Filename &state_filename);
  void optimal_resize();
  void reset_images();
  void generate_images(bool redo_all);
  bool read_stale_eggs(bool redo_all);
  bool write_eggs();

  EggFile *get_egg_file(const std::string &name);
  bool remove_egg_file(const std::string &name);

  void add_command_line_egg(EggFile *egg_file);

  PaletteGroup *get_palette_group(const std::string &name);
  PaletteGroup *test_palette_group(const std::string &name) const;
  PaletteGroup *get_default_group();
  TextureImage *get_texture(const std::string &name);

private:
  static const char *yesno(bool flag);

public:
  static int _pi_version;
  static int _min_pi_version;
  static int _read_pi_version;

  enum RemapUV {
    RU_never,
    RU_group,
    RU_poly,
    RU_invalid
  };

  static RemapUV string_remap(const std::string &str);

  bool _is_valid;

  // These values are not stored in the textures.boo file, but are specific to
  // each session.
  TxaFile _txa_file;
  std::string _default_groupname;
  std::string _default_groupdir;
  bool _noabs;

  // The following parameter values specifically relate to textures and
  // palettes.  These values are stored in the textures.boo file for future
  // reference.
  std::string _generated_image_pattern;
  std::string _map_dirname;
  Filename _shadow_dirname;
  Filename _rel_dirname;
  int _pal_x_size, _pal_y_size;
  LColord _background;
  int _margin;
  bool _omit_solitary;
  bool _omit_everything;
  double _coverage_threshold;
  bool _force_power_2;
  bool _aggressively_clean_mapdir;
  bool _round_uvs;
  double _round_unit;
  double _round_fuzz;
  RemapUV _remap_uv, _remap_char_uv;
  PNMFileType *_color_type;
  PNMFileType *_alpha_type;
  PNMFileType *_shadow_color_type;
  PNMFileType *_shadow_alpha_type;
  EggRenderMode::AlphaMode _cutout_mode;
  double _cutout_ratio;

private:
  typedef pvector<TexturePlacement *> Placements;
  void compute_statistics(std::ostream &out, int indent_level,
                          const Placements &placements) const;

  typedef pmap<std::string, EggFile *> EggFiles;
  EggFiles _egg_files;

  typedef pvector<EggFile *> CommandLineEggs;
  CommandLineEggs _command_line_eggs;

  typedef pset<TextureImage *> CommandLineTextures;
  CommandLineTextures _command_line_textures;

  typedef pmap<std::string, PaletteGroup *> Groups;
  Groups _groups;

  typedef pmap<std::string, TextureImage *> Textures;
  Textures _textures;
  typedef pvector<TextureImage *> TextureConflicts;
  TextureConflicts _texture_conflicts;


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_Palettizer(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // These values are only filled in while reading from the bam file; don't
  // use them otherwise.
  int _num_egg_files;
  int _num_groups;
  int _num_textures;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "Palettizer",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class TxaLine;
};

// This is a global Palettizer pointer that may be filled in when the
// Palettizer is created, for convenience in referencing it from multiple
// places.  (Generally, a standalone program will only create one Palettizer
// object in a session.)
extern Palettizer *pal;

#endif

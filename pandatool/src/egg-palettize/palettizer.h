// Filename: palettizer.h
// Created by:  drose (01Dec00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PALETTIZER_H
#define PALETTIZER_H

#include "pandatoolbase.h"

#include "txaFile.h"

#include "typedWritable.h"

#include "pvector.h"
#include "pset.h"
#include "pmap.h"

class PNMFileType;
class EggFile;
class PaletteGroup;
class TextureImage;
class TexturePlacement;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : Palettizer
// Description : This is the main engine behind egg-palettize.  It
//               contains all of the program parameters, from the
//               command line or saved from a previous session, and
//               serves as the driving force in the actual palettizing
//               process.
////////////////////////////////////////////////////////////////////
class Palettizer : public TypedWritable {
public:
  Palettizer();

  void report_pi() const;
  void report_statistics() const;

  void read_txa_file(const Filename &txa_filename);
  void all_params_set();
  void process_command_line_eggs(bool force_texture_read, const Filename &state_filename);
  void process_all(bool force_texture_read, const Filename &state_filename);
  void optimal_resize();
  void reset_images();
  void generate_images(bool redo_all);
  bool read_stale_eggs(bool redo_all);
  bool write_eggs();

  EggFile *get_egg_file(const string &name);
  bool remove_egg_file(const string &name);

  PaletteGroup *get_palette_group(const string &name);
  PaletteGroup *test_palette_group(const string &name) const;
  PaletteGroup *get_default_group();
  TextureImage *get_texture(const string &name);

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

  static RemapUV string_remap(const string &str);

  // These values are not stored in the bam file, but are specific to
  // each session.
  TxaFile _txa_file;
  string _default_groupname;
  string _default_groupdir;

  // The following parameter values specifically relate to textures
  // and palettes.  These values are stored in the bam file for future
  // reference.
  string _map_dirname;
  Filename _shadow_dirname;
  Filename _rel_dirname;
  int _pal_x_size, _pal_y_size;
  int _margin;
  bool _omit_solitary;
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

private:
  typedef pvector<TexturePlacement *> Placements;
  void compute_statistics(ostream &out, int indent_level,
                          const Placements &placements) const;

  typedef pmap<string, EggFile *> EggFiles;
  EggFiles _egg_files;

  typedef pvector<EggFile *> CommandLineEggs;
  CommandLineEggs _command_line_eggs;

  typedef pset<TextureImage *> CommandLineTextures;
  CommandLineTextures _command_line_textures;

  typedef pmap<string, PaletteGroup *> Groups;
  Groups _groups;

  typedef pmap<string, TextureImage *> Textures;
  Textures _textures;


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_Palettizer(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // These values are only filled in while reading from the bam file;
  // don't use them otherwise.
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

  friend class EggPalettize;
  friend class TxaLine;
};

extern Palettizer *pal;

#endif

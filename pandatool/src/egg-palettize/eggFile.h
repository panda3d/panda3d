// Filename: eggFile.h
// Created by:  drose (28Nov00)
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

#ifndef EGGFILE_H
#define EGGFILE_H

#include "pandatoolbase.h"

#include "paletteGroups.h"
#include "textureReference.h"

#include "filename.h"
#include "namable.h"
#include "typedWritable.h"

#include "pset.h"

class SourceTextureImage;
class EggData;
class TextureImage;

////////////////////////////////////////////////////////////////////
//       Class : EggFile
// Description : This represents a single egg file known to the
//               palettizer.  It may reference a number of textures,
//               and may also be assigned to a number of groups.  All
//               of its textures will try to assign themselves to one
//               of its groups.
////////////////////////////////////////////////////////////////////
class EggFile : public TypedWritable, public Namable {
public:
  EggFile();

  void from_command_line(EggData *data,
                         const Filename &source_filename,
                         const Filename &dest_filename);

  void scan_textures();
  void get_textures(pset<TextureImage *> &result) const;

  void pre_txa_file();
  void match_txa_groups(const PaletteGroups &groups);
  void post_txa_file();

  const PaletteGroups &get_explicit_groups() const;
  PaletteGroup *get_default_group() const;
  const PaletteGroups &get_complete_groups() const;
  void clear_surprise();
  bool is_surprise() const;

  void mark_stale();
  bool is_stale() const;

  void build_cross_links();
  void choose_placements();

  bool has_data() const;

  void update_egg();
  void remove_egg();
  bool read_egg();
  bool write_egg();

  void write_description(ostream &out, int indent_level = 0) const;
  void write_texture_refs(ostream &out, int indent_level = 0) const;

private:
  EggData *_data;
  Filename _current_directory;
  Filename _source_filename;
  Filename _dest_filename;

  typedef pvector<TextureReference *> Textures;
  Textures _textures;

  bool _first_txa_match;
  PaletteGroups _explicitly_assigned_groups;
  PaletteGroup *_default_group;
  PaletteGroups _complete_groups;
  bool _is_surprise;
  bool _is_stale;


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_EggFile(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // This value is only filled in while reading from the bam file;
  // don't use it otherwise.
  int _num_textures;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    Namable::init_type();
    register_type(_type_handle, "EggFile",
                  TypedWritable::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif


// Filename: textureImage.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREIMAGE_H
#define TEXTUREIMAGE_H

#include <pandatoolbase.h>

#include "imageFile.h"
#include "paletteGroups.h"
#include "textureRequest.h"

#include <namable.h>
#include <filename.h>
#include <pnmImage.h>

#include <map>
#include <set>

class SourceTextureImage;
class DestTextureImage;
class TexturePlacement;
class EggFile;

////////////////////////////////////////////////////////////////////
// 	 Class : TextureImage
// Description : This represents a single source texture that is
//               referenced by one or more egg files.  It may be
//               assigned to multiple PaletteGroups, and thus placed
//               on multiple PaletteImages (up to one per
//               PaletteGroup).
//
//               Since a TextureImage may be referenced by multiple
//               egg files that are each assigned to a different set
//               of groups, it tries to maximize sharing between egg
//               files and minimize the number of different
//               PaletteGroups it is assigned to.
////////////////////////////////////////////////////////////////////
class TextureImage : public ImageFile, public Namable {
public:
  TextureImage();

  void note_egg_file(EggFile *egg_file);
  void assign_groups();

  const PaletteGroups &get_groups() const;
  TexturePlacement *get_placement(PaletteGroup *group) const;
  void force_replace();

  void pre_txa_file();
  void post_txa_file();
  void determine_placement_size();

  bool get_omit() const;
  double get_coverage_threshold() const;
  int get_margin() const;
  bool is_surprise() const;

  SourceTextureImage *get_source(const Filename &filename, 
				 const Filename &alpha_filename);

  SourceTextureImage *get_preferred_source();

  void copy_unplaced(bool redo_all);

  const PNMImage &read_source_image();
  const PNMImage &get_dest_image();

  void write_source_pathnames(ostream &out, int indent_level = 0) const;
  void write_scale_info(ostream &out, int indent_level = 0);

private:
  typedef set<EggFile *> EggFiles;
  typedef vector<EggFile *> WorkingEggs;
  typedef map<string, SourceTextureImage *> Sources;
  typedef map<string, DestTextureImage *> Dests;

  static int compute_egg_count(PaletteGroup *group, 
			       const WorkingEggs &egg_files);

  void assign_to_groups(const PaletteGroups &groups);
  void consider_grayscale();
  void consider_unalpha();

  void remove_old_dests(const Dests &a, const Dests &b);
  void copy_new_dests(const Dests &a, const Dests &b);

  string get_source_key(const Filename &filename, 
			const Filename &alpha_filename);

private:
  TextureRequest _request;
  TextureProperties _pre_txa_properties;
  SourceTextureImage *_preferred_source;
  bool _is_surprise;
 
  bool _ever_read_image;
  bool _forced_grayscale;
  bool _forced_unalpha;

  PaletteGroups _explicitly_assigned_groups;
  PaletteGroups _actual_assigned_groups;

  EggFiles _egg_files;

  typedef map<PaletteGroup *, TexturePlacement *> Placement;
  Placement _placement;

  Sources _sources;
  Dests _dests;

  bool _read_source_image;
  PNMImage _source_image;
  bool _got_dest_image;
  PNMImage _dest_image;


  // The TypedWriteable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram); 
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

protected:
  static TypedWriteable *make_TextureImage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // These values are only filled in while reading from the bam file;
  // don't use them otherwise.
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


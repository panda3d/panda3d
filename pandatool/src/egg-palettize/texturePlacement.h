// Filename: texturePlacement.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREPLACEMENT_H
#define TEXTUREPLACEMENT_H

#include <pandatoolbase.h>

#include "omitReason.h"
#include "texturePosition.h"

#include <typedWriteable.h>
#include <luse.h>

#include <set>


class TextureImage;
class PaletteGroup;
class PaletteImage;
class PalettePage;
class TextureProperties;
class TextureReference;
class PNMImage;

////////////////////////////////////////////////////////////////////
// 	 Class : TexturePlacement
// Description : This corresponds to a particular assignment of a
//               TextureImage with a PaletteGroup, and specifically
//               describes which PaletteImage (if any), and where on
//               the PaletteImage, the TextureImage has been assigned
//               to.
////////////////////////////////////////////////////////////////////
class TexturePlacement : public TypedWriteable {
private:
  TexturePlacement();

public:
  TexturePlacement(TextureImage *texture, PaletteGroup *group);
  ~TexturePlacement();

  TextureImage *get_texture() const;
  const TextureProperties &get_properties() const;
  PaletteGroup *get_group() const;

  void add_egg(TextureReference *reference);
  void remove_egg(TextureReference *reference);

  bool determine_size();
  OmitReason get_omit_reason() const;
  int get_x_size() const;
  int get_y_size() const;
  double get_uv_area() const;

  bool is_placed() const;
  PaletteImage *get_image() const;
  PalettePage *get_page() const;
  int get_placed_x() const;
  int get_placed_y() const;
  int get_placed_x_size() const;
  int get_placed_y_size() const;
  double get_placed_uv_area() const;

  void place_at(PaletteImage *image, int x, int y);
  void force_replace();
  void omit_solitary();
  void not_solitary();
  bool intersects(int x, int y, int x_size, int y_size);

  void compute_tex_matrix(LMatrix3d &transform);

  void write_placed(ostream &out, int indent_level = 0);

  bool is_filled() const;
  void fill_image(PNMImage &image);
  void flag_error_image(PNMImage &image);

private:
  void compute_size_from_uvs(const TexCoordd &min_uv, const TexCoordd &max_uv);

  TextureImage *_texture;
  PaletteGroup *_group;
  PaletteImage *_image;

  bool _has_uvs;
  bool _size_known;
  TexturePosition _position;

  bool _is_filled;
  TexturePosition _placed;
  OmitReason _omit_reason;

  typedef set<TextureReference *> References;
  References _references;

  // The TypedWriteable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram); 
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

protected:
  static TypedWriteable *make_TexturePlacement(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // This value is only filled in while reading from the bam file;
  // don't use it otherwise.
  int _num_references;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteable::init_type();
    register_type(_type_handle, "TexturePlacement",
		  TypedWriteable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif


// Filename: palette.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef PALETTE_H
#define PALETTE_H

#include <pandatoolbase.h>

#include "imageFile.h"

#include <filename.h>

#include <vector>

class Texture;
class PNMImage;
class AttribFile;

////////////////////////////////////////////////////////////////////
// 	 Class : Palette
// Description : 
////////////////////////////////////////////////////////////////////
class Palette : public ImageFile {
public:
  Palette(const Filename &filename, int xsize, int ysize, int components,
	  AttribFile *af);
  Palette(int index, int xsize, int ysize, int components,
	  AttribFile *af);
  ~Palette();

  virtual Filename get_filename() const;
  virtual Filename get_basename() const;

  bool changed() const;
  bool new_palette() const;
  int get_num_textures() const;

  bool check_uses_alpha() const;

  void get_size(int &xsize, int &ysize) const;

  void place_texture_at(Texture *texture, int left, int top,
			int xsize, int ysize, int margin);

  bool pack_texture(Texture *texture);
  bool unpack_texture(Texture *texture);

  void optimal_resize();

  void finalize_palette();
  
  void write(ostream &out) const;

  bool generate_image();
  bool refresh_image();

private:  
  class TexturePlacement {
  public:
    bool intersects(int left, int top, int xsize, int ysize) const;
    PNMImage *resize_image(PNMImage *source) const;
    PNMImage *add_margins(PNMImage *source) const;

    Texture *_texture;
    int _left, _top;
    int _xsize, _ysize, _margin;
  };

  Palette *try_resize(int new_xsize, int new_ysize) const;
  int get_max_y() const;
  bool find_home(int &left, int &top, int xsize, int ysize) const;
  bool copy_texture_image(PNMImage &palette, const TexturePlacement &tp);

  
  typedef vector<TexturePlacement> TexPlace;
  TexPlace _texplace;

  Filename _filename;
  Filename _basename;
  int _index;
  int _xsize, _ysize, _components;
  bool _palette_changed;
  bool _new_palette;
  
  AttribFile *_attrib_file;
};

#endif

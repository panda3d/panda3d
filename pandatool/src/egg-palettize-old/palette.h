// Filename: palette.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef PALETTE_H
#define PALETTE_H

#include <pandatoolbase.h>

#include <filename.h>

#include <vector>

class PaletteGroup;
class TexturePacking;
class PNMImage;
class AttribFile;

////////////////////////////////////////////////////////////////////
// 	 Class : Palette
// Description : A single palettized image file within a palette
//               group.  This represents one page of all the
//               palettized textures within this group; there might be
//               multiple Palette images within a single group,
//               depending on the number and size of the palettized
//               textures.
////////////////////////////////////////////////////////////////////
class Palette {
public:
  Palette(const Filename &filename, PaletteGroup *group,
	  int xsize, int ysize, int components, AttribFile *attrib_file);
  Palette(PaletteGroup *group, int index,
	  int xsize, int ysize, int components, AttribFile *attrib_file);
  ~Palette();

  Filename get_filename() const;
  bool has_alpha_filename() const;
  Filename get_alpha_filename() const;
  Filename get_basename() const;

  PaletteGroup *get_group() const;

  bool changed() const;
  bool new_palette() const;
  int get_num_textures() const;

  bool check_uses_alpha() const;

  void get_size(int &xsize, int &ysize) const;

  void place_texture_at(TexturePacking *packing, int left, int top,
			int xsize, int ysize, int margin);

  bool pack_texture(TexturePacking *packing);
  bool unpack_texture(TexturePacking *packing);

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

    TexturePacking *_packing;
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
  PaletteGroup *_group;
  int _index;
  int _xsize, _ysize, _components;
  bool _uses_alpha;
  bool _palette_changed;
  bool _new_palette;
  
  AttribFile *_attrib_file;
};

#endif

// Filename: pTexture.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTEXTURE_H
#define PTEXTURE_H

#include <pandatoolbase.h>

#include <filename.h>

#include <set>
#include <map>

class Palette;
class PNMImage;
class AttribFile;
class PaletteGroup;
class TexturePacking;
class TextureEggRef;

////////////////////////////////////////////////////////////////////
// 	 Class : PTexture
// Description : A single texture filename, as read from an egg file
//               or from a .txa file.  This may be considered for
//               palettization on a number of different groups, but it
//               must have the same size in each group.
////////////////////////////////////////////////////////////////////
class PTexture {
public:
  PTexture(AttribFile *attrib_file, const Filename &name);
  ~PTexture();

  Filename get_name() const;
  
  void add_filename(const Filename &filename);

  bool get_size(int &xsize, int &ysize, int &zsize);
  void set_size(int xsize, int ysize, int zsize);

  bool get_req(int &xsize, int &ysize);
  bool get_last_req(int &xsize, int &ysize);
  void set_last_req(int xsize, int ysize);
  void reset_req(int xsize, int ysize);
  void scale_req(double scale_pct);
  void clear_req();
  double get_scale_pct();

  int get_margin() const;
  void set_margin(int margin);

  void user_omit();

  TexturePacking *add_to_group(PaletteGroup *group);
  TexturePacking *check_group(PaletteGroup *group) const;

  void set_changed(bool changed);

  bool matched_anything() const;
  void set_matched_anything(bool matched_anything);
  bool is_unused() const;

  void write_size(ostream &out);
  void write_pathname(ostream &out) const;

  PNMImage *read_image();

  typedef set<TextureEggRef *> Eggs;
  Eggs _eggs;

private:  
  void check_size();
  void read_header();
  bool read_image_header(const Filename &filename, 
			 int &xsize, int &ysize, int &zsize);

  Filename _name;

  typedef set<Filename> Filenames;
  Filenames _filenames;

  bool _got_filename;
  Filename _filename;
  bool _file_exists;
  bool _texture_changed;
  bool _matched_anything;

  bool _got_size;
  int _xsize, _ysize;
  int _zsize;

  bool _got_req;
  int _req_xsize, _req_ysize;
  bool _got_last_req;
  int _last_req_xsize, _last_req_ysize;
  int _margin;
  bool _omit;

  bool _read_header;

  AttribFile *_attrib_file;

  typedef map<PaletteGroup *, TexturePacking *> Packing;
  Packing _packing;

  friend class TexturePacking;
};


#endif

// Filename: pTexture.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef PTEXTURE_H
#define PTEXTURE_H

#include <pandatoolbase.h>

#include "imageFile.h"

#include <set>

class Palette;
class PNMImage;
class AttribFile;

////////////////////////////////////////////////////////////////////
// 	 Class : PTexture
// Description : 
////////////////////////////////////////////////////////////////////
class PTexture : public ImageFile {
public:
  enum OmitReason {
    OR_none,
    OR_size, OR_repeats, OR_omitted, OR_unused, OR_unknown,
    OR_cmdline, OR_solitary
  };

  PTexture(AttribFile *af, const Filename &name);

  Filename get_name() const;
  
  void add_filename(const Filename &filename);

  virtual Filename get_filename() const;
  virtual Filename get_basename() const;

  bool get_size(int &xsize, int &ysize);
  void set_size(int xsize, int ysize);

  bool get_req(int &xsize, int &ysize);
  bool get_last_req(int &xsize, int &ysize);
  void set_last_req(int xsize, int ysize);
  void reset_req(int xsize, int ysize);
  void scale_req(double scale_pct);
  void clear_req();
  double get_scale_pct();

  int get_margin() const;
  void set_margin(int margin);

  OmitReason get_omit() const;
  void set_omit(OmitReason omit);

  bool needs_refresh();
  void set_changed(bool changed);

  bool unused() const;
  void set_unused(bool unused);

  bool matched_anything() const;
  void set_matched_anything(bool matched_anything);

  bool uses_alpha() const;
  void set_uses_alpha(bool uses_alpha);

  void mark_pack_location(Palette *palette, int left, int top,
			  int xsize, int ysize, int margin);
  void mark_unpacked();
  bool is_packed() const;
  bool is_really_packed() const;
  Palette *get_palette() const;
  bool get_packed_location(int &left, int &top) const;
  bool get_packed_size(int &xsize, int &ysize, int &margin) const;
  void record_orig_state();
  bool packing_changed() const;

  void write_size(ostream &out);
  void write_pathname(ostream &out) const;
  void write_unplaced(ostream &out) const;

  bool transfer();

  PNMImage *read_image();

private:  
  void check_size();
  void read_header();
  bool read_image_header(const Filename &filename, int &xsize, int &ysize);
  static int to_power_2(int value);

  Filename _name;

  typedef set<Filename> Filenames;
  Filenames _filenames;

  bool _got_filename;
  Filename _filename;
  bool _file_exists;
  bool _texture_changed;
  bool _unused;
  bool _matched_anything;
  bool _uses_alpha;

  bool _got_size;
  int _xsize, _ysize;

  bool _got_req;
  int _req_xsize, _req_ysize;
  bool _got_last_req;
  int _last_req_xsize, _last_req_ysize;
  int _margin;
  OmitReason _omit;

  bool _is_packed;
  Palette *_palette;
  int _pleft, _ptop, _pxsize, _pysize, _pmargin;

  bool _orig_is_packed;
  Filename _orig_palette_name;
  int _opleft, _optop, _opxsize, _opysize, _opmargin;

  bool _read_header;

  AttribFile *_attrib_file;
};


#endif

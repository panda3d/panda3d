// Filename: texturePacking.h
// Created by:  drose (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREPACKING_H
#define TEXTUREPACKING_H

#include <pandatoolbase.h>

#include "textureOmitReason.h"

#include <filename.h>

class PTexture;
class PaletteGroup;
class Palette;
class AttribFile;

////////////////////////////////////////////////////////////////////
// 	 Class : TexturePacking
// Description : This structure defines how a particular texture is
//               packed into a Palette image, or even whether it is
//               packed at all.
////////////////////////////////////////////////////////////////////
class TexturePacking {
public:
  TexturePacking(PTexture *texture, PaletteGroup *group);
  ~TexturePacking();

  PTexture *get_texture() const;
  PaletteGroup *get_group() const;

  TextureOmitReason get_omit() const;
  void set_omit(TextureOmitReason omit);

  bool unused() const;
  void set_unused(bool unused);

  bool uses_alpha() const;
  void set_uses_alpha(bool uses_alpha);

  bool pack();
  bool unpack();
  bool prepare_repack(bool &optimal);

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

  void set_changed(bool changed);
  bool needs_refresh();

  void write_unplaced(ostream &out) const;

  Filename get_new_filename() const;
  Filename get_old_filename() const;
  bool transfer();

private:
  static int to_power_2(int value);

  AttribFile *_attrib_file;
  PTexture *_texture;
  PaletteGroup *_group;
  TextureOmitReason _omit;
  bool _unused;
  bool _uses_alpha;

  bool _is_packed;
  Palette *_palette;
  int _pleft, _ptop, _pxsize, _pysize, _pmargin;

  bool _orig_is_packed;
  Filename _orig_palette_name;
  int _opleft, _optop, _opxsize, _opysize, _opmargin;

  bool _packing_changed;
};

#endif

// Filename: layeredImage.h
// Created by:  drose (29Nov99)
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

#ifndef LAYEREDIMAGE_H
#define LAYEREDIMAGE_H

#include "pandaappbase.h"

#include "luse.h"
#include "filename.h"

#include <vector>

//#include <stdint.h>

class PNMImage;

class LayeredImage {
public:
  typedef char int8_t;
  typedef long int32_t;

  LayeredImage(int xsize, int ysize);
  ~LayeredImage();

  void add_layer(const string &name, const LVector2d &offset,
                 PNMImage *data);

  bool write_file(const Filename &filename);

  bool write_xcf(ostream &out);

private:
  // XCF property types.  From Gimp's xcf.c.
  enum PropType {
    PROP_END = 0,
    PROP_COLORMAP = 1,
    PROP_ACTIVE_LAYER = 2,
    PROP_ACTIVE_CHANNEL = 3,
    PROP_SELECTION = 4,
    PROP_FLOATING_SELECTION = 5,
    PROP_OPACITY = 6,
    PROP_MODE = 7,
    PROP_VISIBLE = 8,
    PROP_LINKED = 9,
    PROP_PRESERVE_TRANSPARENCY = 10,
    PROP_APPLY_MASK = 11,
    PROP_EDIT_MASK = 12,
    PROP_SHOW_MASK = 13,
    PROP_SHOW_MASKED = 14,
    PROP_OFFSETS = 15,
    PROP_COLOR = 16,
    PROP_COMPRESSION = 17,
    PROP_GUIDES = 18
  };

  class Layer {
  public:
    bool trim();

    string _name;
    LVector2d _offset;
    PNMImage *_data;
  };

  class TileManager {
  public:
    TileManager(const PNMImage *image, int channel);
    int get_nlevels() const;
    int get_level_width(int level) const;
    int get_level_height(int level) const;
    int get_ntiles(int level) const;
    int get_tile_left(int level, int tile) const;
    int get_tile_top(int level, int tile) const;
    int get_tile_width(int level, int tile) const;
    int get_tile_height(int level, int tile) const;

    const PNMImage *_data;
    int _channel;

  private:
    class Level {
    public:
      int _width;
      int _height;
      int _ntile_rows;
      int _ntile_cols;
    };
    typedef vector<Level> Levels;
    Levels _levels;
  };

  int xcf_write_int8(const int8_t *data, int num);
  int xcf_write_int32(const int32_t *data, int num);
  int xcf_write_string(const string &str);
  void xcf_save_image_props();
  void xcf_save_layer_props(const Layer &layer);
  void xcf_save_channel_props();
  void xcf_save_prop(PropType prop_type, ...);
  void xcf_save_layer(const Layer &layer);
  void xcf_save_channel(const string &name, const PNMImage *image,
                        int channel);
  void xcf_save_hierarchy(const PNMImage *image, int channel);
  void xcf_save_level(const TileManager &tm, int level);
  void xcf_save_tile(const TileManager &tm, int level, int tile);
  void xcf_seek_pos(int to_pos);
  void xcf_seek_end();

  typedef vector<Layer> Layers;
  Layers _layers;
  int _xsize;
  int _ysize;

  ostream *_out;
  int _pos;
};

#endif

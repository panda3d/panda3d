// Filename: layeredImage.cxx
// Created by:  drose (29Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "layeredImage.h"

#include "pnmImage.h"
#include "datagram.h"

#include <stdarg.h>

// Constants taken from various header files in Gimp.
#define TILE_WIDTH 64
#define TILE_HEIGHT 64

#define RGB_GIMAGE  0
#define RGBA_GIMAGE 1

LayeredImage::TileManager::
TileManager(const PNMImage *image, int channel) :
  _data(image), _channel(channel)
{
  int width = image->get_x_size();
  int height = image->get_y_size();

  while (width > TILE_WIDTH || height > TILE_WIDTH) {
    _levels.push_back(Level());
    Level &l = _levels.back();

    l._width = width;
    l._height = height;
    l._ntile_rows = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
    l._ntile_cols = (width + TILE_WIDTH - 1) / TILE_WIDTH;

    width /= 2;
    height /= 2;
  }

  _levels.push_back(Level());
  Level &l = _levels.back();

  l._width = width;
  l._height = height;
  l._ntile_rows = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
  l._ntile_cols = (width + TILE_WIDTH - 1) / TILE_WIDTH;
}

int LayeredImage::TileManager::
get_nlevels() const {
  return _levels.size();
}

int LayeredImage::TileManager::
get_level_width(int level) const {
  assert(level >= 0 && level < (int)_levels.size());
  return _levels[level]._width;
}

int LayeredImage::TileManager::
get_level_height(int level) const {
  assert(level >= 0 && level < (int)_levels.size());
  return _levels[level]._height;
}

int LayeredImage::TileManager::
get_ntiles(int level) const {
  assert(level >= 0 && level < (int)_levels.size());
  return _levels[level]._ntile_rows * _levels[level]._ntile_cols;
}

int LayeredImage::TileManager::
get_tile_left(int level, int tile) const {
  //  int ntile_rows = _levels[level]._ntile_rows;
  int ntile_cols = _levels[level]._ntile_cols;

  //  int r = tile / ntile_cols;
  int c = tile % ntile_cols;
  return c * TILE_WIDTH;
}

int LayeredImage::TileManager::
get_tile_top(int level, int tile) const {
  //  int ntile_rows = _levels[level]._ntile_rows;
  int ntile_cols = _levels[level]._ntile_cols;

  int r = tile / ntile_cols;
  //  int c = tile % ntile_cols;
  return r * TILE_HEIGHT;
}

int LayeredImage::TileManager::
get_tile_width(int level, int tile) const {
  return min(TILE_WIDTH, _data->get_x_size() - get_tile_left(level, tile));
}

int LayeredImage::TileManager::
get_tile_height(int level, int tile) const {
  return min(TILE_HEIGHT, _data->get_y_size() - get_tile_top(level, tile));
}

// Trims off the invisible (alpha-0) border around the layer.  Returns
// true if there is anything left, false if the layer would be empty.
bool LayeredImage::Layer::
trim() {
  assert(_data != NULL);
  if (_data->has_alpha()) {
    int xsize = _data->get_x_size();
    int ysize = _data->get_y_size();

    int top = xsize - 1;
    int left = ysize - 1;
    int bottom = 0;
    int right = 0;

    for (int y = 0; y < ysize; y++) {
      for (int x = 0; x < xsize; x++) {
        if (_data->get_alpha_val(x, y) != 0) {
          top = min(top, y);
          left = min(left, x);
          bottom = max(bottom, y);
          right = max(right, x);
        }
      }
    }

    if (top > bottom || left > right) {
      // The layer is completely empty.
      return false;
    }

    if (top > 0 || left > 0 || bottom < ysize - 1 || right < xsize - 1) {
      xsize = right - left + 1;
      ysize = bottom - top + 1;
      PNMImage *sub = new PNMImage(xsize, ysize, 4);
      sub->copy_sub_image(*_data, 0, 0, left, top);
      delete _data;
      _data = sub;
      _offset[0] += left;
      _offset[1] += top;
    }
  }

  return true;
}

LayeredImage::
LayeredImage(int xsize, int ysize) :
  _xsize(xsize), _ysize(ysize) {
}

LayeredImage::
~LayeredImage() {
  Layers::const_iterator li;
  for (li = _layers.begin(); li != _layers.end(); ++li) {
    delete (*li)._data;
  }
}

void LayeredImage::
add_layer(const string &name, const LVector2d &offset,
          PNMImage *data) {
  _layers.push_back(Layer());
  Layer &l = _layers.back();

  l._name = name;
  l._offset = offset;
  l._data = data;

  if (!l.trim()) {
    // If trimming the layer reveals that it is empty, delete it.
    delete l._data;
    _layers.pop_back();
  }
}

bool LayeredImage::
write_file(const Filename &filename) {
  ofstream out(filename.c_str());

  // Maybe in the future, if we support more than one kind of file
  // here, we'll decide based on the filename extension which kind to
  // write out.

  return write_xcf(out);
}

bool LayeredImage::
write_xcf(ostream &out) {
  _out = &out;
  _pos = 0;

  // Write out the version tag
  static const int version_tag_len = 14;
  int8_t version_tag[version_tag_len];
  memset(version_tag, 0, version_tag_len);
  strcpy((char *)version_tag, "gimp xcf file");
  xcf_write_int8(version_tag, version_tag_len);

  // Write out the width, height, and type.
  int32_t width = _xsize;
  int32_t height = _ysize;
  int32_t base_type = RGB_GIMAGE;
  xcf_write_int32(&width, 1);
  xcf_write_int32(&height, 1);
  xcf_write_int32(&base_type, 1);

  xcf_save_image_props();

  // Save the current file position; we'll return here to place the
  // layer offset information.
  int saved_pos = _pos;

  int nlayers = _layers.size();
  int nchannels = 0;

  // Seek to after the offset lists.
  xcf_seek_pos(_pos + (nlayers + nchannels + 2) * 4);

  // Write out each layer.  Since the layers were added to the
  // LayeredImage object from the bottom up (to me, the intuitive
  // order), and since they are stored in the XCF file from the top
  // down, we must reverse the order here.
  Layers::reverse_iterator li;
  for (li = _layers.rbegin(); li != _layers.rend(); ++li) {
    int32_t offset = _pos;
    xcf_save_layer(*li);

    // Go back to write this layer offset.
    xcf_seek_pos(saved_pos);
    xcf_write_int32(&offset, 1);
    saved_pos = _pos;

    xcf_seek_end();
  }

  // Write out '0' offset to indicate the end of the layer offsets.
  int32_t offset = 0;
  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
  saved_pos = _pos;
  xcf_seek_end();

  /*
    No need to explicitly write out the channels.

  // Write out each channel.
  static const char *channel_name[3] = { "red", "green", "blue" };
  for (int i = 0; i < 3; i++) {
    // save the start offset of where we are writing
    // out the next channel.
    int32_t offset = _pos;

    // write out the channel.
    xcf_save_channel(channel_name[i], _layers.front()._data, i);

    // seek back to where we are to write out the next
    // channel offset and write it out.
    xcf_seek_pos(saved_pos);
    xcf_write_int32(&offset, 1);

    // increment the location we are to write out the
    // next offset.
    saved_pos = _pos;

    // seek to the end of the file which is where
    // we will write out the next channel.
    xcf_seek_end();
  }
  */

  // Write out '0' offset to indicate the end of the channel offsets.
  offset = 0;
  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
  saved_pos = _pos;
  xcf_seek_end();

  return !_out->fail();
}

int LayeredImage::
xcf_write_int8(const int8_t *data, int num) {
  _out->write((const char *)data, num);
  return _pos += num;
}

int LayeredImage::
xcf_write_int32(const int32_t *data, int num) {
  // We need to write a bunch of big-endian int32's.
  Datagram dg;
  for (int i = 0; i < num; i++) {
    dg.add_be_int32(data[i]);
  }
  _out->write((const char *)dg.get_data(), dg.get_length());
  return _pos += dg.get_length();
}

int LayeredImage::
xcf_write_string(const string &str) {
  int32_t size = (int32_t)str.size() + 1;
  if (str.empty()) {
    size = 0;
  }
  xcf_write_int32(&size, 1);
  return xcf_write_int8((const int8_t *)str.c_str(), size);
}

void LayeredImage::
xcf_save_image_props() {
  xcf_save_prop(PROP_END);
}

void LayeredImage::
xcf_save_layer_props(const LayeredImage::Layer &layer) {
  if (&layer == &_layers.front()) {
    xcf_save_prop(PROP_ACTIVE_LAYER);
  }
  xcf_save_prop(PROP_OPACITY, 255);
  xcf_save_prop(PROP_VISIBLE, 1);
  xcf_save_prop(PROP_LINKED, 0);
  xcf_save_prop(PROP_PRESERVE_TRANSPARENCY, 0);
  xcf_save_prop(PROP_APPLY_MASK, 1);
  xcf_save_prop(PROP_EDIT_MASK, 0);
  xcf_save_prop(PROP_SHOW_MASK, 0);
  xcf_save_prop(PROP_MODE, 0);

  xcf_save_prop(PROP_OFFSETS,
                (int32_t)layer._offset[0],
                (int32_t)layer._offset[1]);
  xcf_save_prop(PROP_END);
}

void LayeredImage::
xcf_save_channel_props() {
  xcf_save_prop(PROP_OPACITY, 255);
  xcf_save_prop(PROP_VISIBLE, 1);
  xcf_save_prop(PROP_SHOW_MASKED, 0);
  //  xcf_save_prop(PROP_COLOR, channel->col);

  xcf_save_prop(PROP_END);
}

// This odd function is lifted from Gimp's xcf.c.
void LayeredImage::
xcf_save_prop(LayeredImage::PropType prop_type, ...) {
  int32_t size;
  va_list args;

  va_start(args, prop_type);

  switch (prop_type) {
  case PROP_END:
    size = 0;

    xcf_write_int32((int32_t*)&prop_type, 1);
    xcf_write_int32(&size, 1);
    break;
  case PROP_COLORMAP:
    {
      int32_t ncolors;
      int8_t *colors;

      ncolors = va_arg(args, int32_t);
      colors = va_arg(args, int8_t*);
      size = 4 + ncolors;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&ncolors, 1);
      xcf_write_int8(colors, ncolors * 3);
    }
    break;
  case PROP_ACTIVE_LAYER:
  case PROP_ACTIVE_CHANNEL:
  case PROP_SELECTION:
    size = 0;

    xcf_write_int32((int32_t*)&prop_type, 1);
    xcf_write_int32(&size, 1);
    break;
  case PROP_FLOATING_SELECTION:
    assert(false);
    break;
  case PROP_OPACITY:
    {
      int32_t opacity;

      opacity = va_arg(args, int32_t);

      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32((int32_t*)&opacity, 1);
    }
    break;
  case PROP_MODE:
    {
      int32_t mode;

      mode = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32((int32_t*)&mode, 1);
    }
    break;
  case PROP_VISIBLE:
    {
      int32_t visible;

      visible = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&visible, 1);
    }
    break;
  case PROP_LINKED:
    {
      int32_t linked;

      linked = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&linked, 1);
    }
    break;
  case PROP_PRESERVE_TRANSPARENCY:
    {
      int32_t preserve_trans;

      preserve_trans = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&preserve_trans, 1);
    }
    break;
  case PROP_APPLY_MASK:
    {
      int32_t apply_mask;

      apply_mask = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&apply_mask, 1);
    }
    break;
  case PROP_EDIT_MASK:
    {
      int32_t edit_mask;

      edit_mask = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&edit_mask, 1);
    }
    break;
  case PROP_SHOW_MASK:
    {
      int32_t show_mask;

      show_mask = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&show_mask, 1);
    }
    break;
  case PROP_SHOW_MASKED:
    {
      int32_t show_masked;

      show_masked = va_arg(args, int32_t);
      size = 4;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32(&show_masked, 1);
    }
    break;
  case PROP_OFFSETS:
    {
      int32_t offsets[2];

      offsets[0] = va_arg(args, int32_t);
      offsets[1] = va_arg(args, int32_t);
      size = 8;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int32((int32_t*) offsets, 2);
    }
    break;
  case PROP_COLOR:
    {
      int8_t *color;

      color = va_arg(args, int8_t*);
      size = 3;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int8(color, 3);
    }
    break;
  case PROP_COMPRESSION:
    {
      int8_t compression;

      compression =(int8_t) va_arg(args, int32_t);
      size = 1;

      xcf_write_int32((int32_t*)&prop_type, 1);
      xcf_write_int32(&size, 1);
      xcf_write_int8(&compression, 1);
    }
    break;
  case PROP_GUIDES:
    assert(false);
    break;
  }

  va_end(args);
}

void LayeredImage::
xcf_save_layer(const LayeredImage::Layer &layer) {
  // write out the width, height and image type information for the layer
  int32_t width = layer._data->get_x_size();
  int32_t height = layer._data->get_y_size();
  int32_t type = RGBA_GIMAGE;
  xcf_write_int32((int32_t*)&width, 1);
  xcf_write_int32((int32_t*)&height, 1);
  xcf_write_int32((int32_t*)&type, 1);

  // write out the layer's name
  xcf_write_string(layer._name);

  // write out the layer properties
  xcf_save_layer_props(layer);

  // save the current position which is where the hierarchy offset
  //  will be stored.
  int saved_pos = _pos;

  // write out the layer tile hierarchy
  xcf_seek_pos(_pos + 8);
  int32_t offset = _pos;

  xcf_save_hierarchy(layer._data, -1);

  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
  saved_pos = _pos;

  // write out the layer mask.  We write out the alpha channel here
  // instead of as a proper alpha channel, since it's more convenient
  // in The Gimp to edit the alpha channel in the layer mask.

  if (layer._data->has_alpha()) {
    xcf_seek_end();
    offset = _pos;
    xcf_save_channel("mask", layer._data, 3);
  } else {
    offset = 0;
  }

  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
}

void LayeredImage::
xcf_save_channel(const string &name, const PNMImage *image, int channel) {
  int32_t saved_pos;
  int32_t offset;

  // write out the width and height information for the channel
  int32_t width = image->get_x_size();
  int32_t height = image->get_y_size();
  xcf_write_int32(&width, 1);
  xcf_write_int32(&height, 1);

  // write out the channels name
  xcf_write_string(name);

  // write out the channel properties
  xcf_save_channel_props();

  // save the current position which is where the hierarchy offset
  //  will be stored.
  saved_pos = _pos;

  /* write out the channel tile hierarchy */
  xcf_seek_pos(_pos + 4);
  offset = _pos;

  xcf_save_hierarchy(image, channel);

  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
  saved_pos = _pos;
}

void LayeredImage::
xcf_save_hierarchy(const PNMImage *image, int channel) {
  int32_t width = image->get_x_size();
  int32_t height = image->get_y_size();
  int32_t bpp = (channel < 0) ? 4 : 1;
  xcf_write_int32(&width, 1);
  xcf_write_int32(&height, 1);
  xcf_write_int32(&bpp, 1);

  int saved_pos = _pos;

  TileManager tm(image, channel);
  int nlevels = tm.get_nlevels();

  xcf_seek_pos(_pos + (nlevels + 1) * 4);

  for (int i = 0; i < nlevels; i++) {
    // save the start offset of where we are writing
    // out the next level.
    int32_t offset = _pos;

    // write out the level.
    xcf_save_level(tm, i);

    // seek back to where we are to write out the next
    // level offset and write it out.
    xcf_seek_pos(saved_pos);
    xcf_write_int32(&offset, 1);

    // increment the location we are to write out the
    // next offset.
    saved_pos = _pos;

    // seek to the end of the file which is where
    // we will write out the next level.
    xcf_seek_end();
  }

  // write out a '0' offset position to indicate the end
  // of the level offsets.
  int32_t offset = 0;
  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
}

void LayeredImage::
xcf_save_level(const LayeredImage::TileManager &tm, int level) {
  // write out the width and height information for the channel
  int32_t width = tm.get_level_width(level);
  int32_t height = tm.get_level_height(level);
  xcf_write_int32(&width, 1);
  xcf_write_int32(&height, 1);

  int saved_pos = _pos;

  int ntiles = tm.get_ntiles(level);
  xcf_seek_pos(_pos + (ntiles + 1) * 4);

  for (int i = 0; i < ntiles; i++) {
    // save the start offset of where we are writing
    // out the next tile.
    int32_t offset = _pos;

    // write out the tile.
    xcf_save_tile(tm, level, i);

    // seek back to where we are to write out the next
    // tile offset and write it out.
    xcf_seek_pos(saved_pos);
    xcf_write_int32(&offset, 1);

    // increment the location we are to write out the
    // next offset.
    saved_pos = _pos;

    xcf_seek_end();
  }

  // write out a '0' offset position to indicate the end
  // of the level offsets.
  int32_t offset = 0;
  xcf_seek_pos(saved_pos);
  xcf_write_int32(&offset, 1);
}

void LayeredImage::
xcf_save_tile(const LayeredImage::TileManager &tm, int level, int tile) {
  int xoff = tm.get_tile_left(level, tile);
  int yoff = tm.get_tile_top(level, tile);
  int xsize = tm.get_tile_width(level, tile);
  int ysize = tm.get_tile_height(level, tile);

  if (tm._channel < 0) {
    int size = xsize * ysize * 4;
    int8_t *array = new int8_t[size];
    int i = 0;
    for (int y = yoff; y < yoff + ysize; y++) {
      for (int x = xoff; x < xoff + xsize; x++) {
        array[i++] = tm._data->get_red_val(x, y);
        array[i++] = tm._data->get_green_val(x, y);
        array[i++] = tm._data->get_blue_val(x, y);
        array[i++] = -1;
      }
    }
    assert(i == size);
    xcf_write_int8(array, size);
    delete[] array;

  } else {
    int size = xsize * ysize;
    int8_t *array = new int8_t[size];
    int i = 0;
    for (int y = yoff; y < yoff + ysize; y++) {
      for (int x = xoff; x < xoff + xsize; x++) {
        array[i++] = tm._data->get_channel_val(x, y, tm._channel);
      }
    }
    assert(i == size);
    xcf_write_int8(array, size);
    delete[] array;
  }
}

void LayeredImage::
xcf_seek_pos(int to_pos) {
  _out->seekp(to_pos);
  _pos = to_pos;
}

void LayeredImage::
xcf_seek_end() {
  _out->seekp(0, ios::end);
  _pos = _out->tellp();
}

// Filename: texture.h
// Created by:  mike (09Jan97)
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
#ifndef TEXTURE_H
#define TEXTURE_H

#include "pandabase.h"

#include "imageBuffer.h"
#include "pixelBuffer.h"
#include "graphicsStateGuardianBase.h"
#include "pmap.h"

class PNMImage;
class TextureContext;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : Texture
// Description : 2D texture class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Texture : public ImageBuffer {
PUBLISHED:
  enum FilterType {
    // Mag Filter and Min Filter

    // Point sample the pixel
    FT_nearest,

    // Bilinear filtering of four neighboring pixels
    FT_linear,

    // Min Filter Only

    // Point sample the pixel from the nearest mipmap level
    FT_nearest_mipmap_nearest,

    // Bilinear filter the pixel from the nearest mipmap level
    FT_linear_mipmap_nearest,

    // Point sample the pixel from two mipmap levels, and linearly blend
    FT_nearest_mipmap_linear,

    // A.k.a. trilinear filtering: Bilinear filter the pixel from
    // two mipmap levels, and linearly blend the results.
    FT_linear_mipmap_linear,

    // Returned by string_filter_type() for an invalid match.
    FT_invalid
  };

  enum WrapMode {
    WM_clamp,  // coords that would be outside [0-1] are clamped to 0 or 1
    WM_repeat,
    WM_mirror,
    WM_mirror_once,   // mirror once, then clamp
    WM_border_color,  // coords outside [0-1] use explict border color
    // Returned by string_wrap_mode() for an invalid match.
    WM_invalid
  };

PUBLISHED:
  Texture();
  Texture(int xsize, int ysize, int components, int component_width, PixelBuffer::Type type, PixelBuffer::Format format,
          bool bAllocateRAM);
  ~Texture();

  bool read(const Filename &fullpath, int num_components = 0);
  bool read(const Filename &fullpath, const Filename &alpha_fullpath,
            int num_components = 0);
  bool write(const Filename &fullpath = "") const;

  void set_wrapu(WrapMode wrap);
  void set_wrapv(WrapMode wrap);
  void set_minfilter(FilterType filter);
  void set_magfilter(FilterType filter);
  void set_anisotropic_degree(int anisotropic_degree);
  void set_border_color(const Colorf &color);

  INLINE WrapMode get_wrapu() const;
  INLINE WrapMode get_wrapv() const;
  INLINE FilterType get_minfilter() const;
  INLINE FilterType get_magfilter() const;
  INLINE int get_anisotropic_degree() const;
  INLINE bool uses_mipmaps() const;

public:
  bool load(const PNMImage &pnmimage);
  bool store(PNMImage &pnmimage) const;

  static bool is_mipmap(FilterType type);

  TextureContext *prepare(GraphicsStateGuardianBase *gsg);
  void unprepare();
  void unprepare(GraphicsStateGuardianBase *gsg);
  void clear_gsg(GraphicsStateGuardianBase *gsg);

  INLINE bool has_ram_image() const;
  PixelBuffer *get_ram_image();
  INLINE void set_keep_ram_image(bool keep_ram_image);
  INLINE bool get_keep_ram_image() const;

  INLINE void apply(GraphicsStateGuardianBase *gsg);

  virtual void copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr);
  virtual void copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
                    const RenderBuffer &rb);

  // These bits are used as parameters to Texture::mark_dirty() and
  // also TextureContext::mark_dirty() (and related functions in
  // TextureContext).
  enum DirtyFlags {
    DF_image      = 0x001,  // The image pixels have changed.
    DF_wrap       = 0x002,  // The wrap properties have changed.
    DF_filter     = 0x004,  // The minfilter or magfilter have changed.
    DF_mipmap     = 0x008,  // The use of mipmaps or not has changed.
  };

  void mark_dirty(int flags_to_set);

  static WrapMode string_wrap_mode(const string &string);
  static FilterType string_filter_type(const string &string);

private:
  WrapMode _wrapu;
  WrapMode _wrapv;
  FilterType _minfilter;
  FilterType _magfilter;
  int _anisotropic_degree;
  bool _keep_ram_image;
  Colorf _border_color;

  // A Texture keeps a list (actually, a map) of all the GSG's that it
  // has been prepared into.  Each GSG conversely keeps a list (a set)
  // of all the Textures that have been prepared there.  When either
  // destructs, it removes itself from the other's list.
  typedef pmap<GraphicsStateGuardianBase *, TextureContext *> Contexts;
  Contexts _contexts;

  // This value represents the intersection of all the dirty flags of
  // the various TextureContexts that might be associated with this
  // texture.
  int _all_dirty_flags;

public:
  // These are public to allow direct manipulation of the underlying
  // pixel buffer when needed.  Know what you are doing!
  PT(PixelBuffer) _pbuffer;

/*
  // If you request a region from the framebuffer that is not a power of 2,
  // we need to grab a larger region that is a power of 2 that contains the
  // requested region and set the pixel buffer size accordingly.  We store
  // the size you requested in the members below.
  bool _has_requested_size;
  int _requested_w;
  int _requested_h;
*/


  // Datagram stuff
public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_Texture(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageBuffer::init_type();
    register_type(_type_handle, "Texture",
                  ImageBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

  friend class TextureContext;
};

#include "texture.I"

#endif


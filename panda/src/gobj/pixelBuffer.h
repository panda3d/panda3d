// Filename: pixelBuffer.h
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
#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include "pandabase.h"

#include "imageBuffer.h"

#include "pnmImage.h"
#include "graphicsStateGuardianBase.h"
#include "pta_uchar.h"

class RenderBuffer;
class Filename;

////////////////////////////////////////////////////////////////////
//       Class : PixelBuffer
// Description : Maintains an array of pixel data corresponding to an
//               image, e.g. copied from the frame buffer, or as part
//               of a Texture.
//
//               Pixel data is stored in a generic, uncompressed
//               format.  Each row of pixels is laid out horizontally,
//               from the top to the bottom, with no padding between
//               rows.  Each pixel consumes one or more bytes,
//               according to get_component_width().  If the Format
//               indicates multiple components are present, they are
//               stored in the order B, G, R, A.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PixelBuffer : public ImageBuffer {
public:
  enum Type {
    T_unsigned_byte,
    T_unsigned_short,
    T_unsigned_byte_332,
    T_float,
  };

  enum Format {
    F_color_index,
    F_stencil_index,
    F_depth_component,
    F_red,
    F_green,
    F_blue,
    F_alpha,
    F_rgb,     // any suitable RGB mode, whatever the hardware prefers
    F_rgb5,    // specifically, 5 bits per R,G,B channel.  
               // this is paired with T_unsigned_byte.  really T_unsigned_byte
               // should not be specified for this one, it should use
               // T_unsigned_5bits or something
    F_rgb8,    // 8 bits per R,G,B channel
    F_rgb12,   // 12 bits per R,G,B channel
    F_rgb332,  // 3 bits per R & G, 2 bits for B
    F_rgba,    // any suitable RGBA mode, whatever the hardware prefers
    F_rgbm,    // as above, but only requires 1 bit for alpha (i.e. mask)
    F_rgba4,   // 4 bits per R,G,B,A channel
    F_rgba5,   // 5 bits per R,G,B channel, 1 bit alpha
    F_rgba8,   // 8 bits per R,G,B,A channel
    F_rgba12,  // 12 bits per R,G,B,A channel
    F_luminance,
    F_luminance_alpha,      // 8 bits luminance, 8 bits alpha
    F_luminance_alphamask   // 8 bits luminance, only needs 1 bit of alpha
  };

  PixelBuffer(void);
  PixelBuffer(int xsize, int ysize, int components,
              int component_width, Type type, Format format);
  PixelBuffer(int xsize, int ysize, int components,
              int component_width, Type type, Format format,
              bool bAllocateRAM);
  PixelBuffer(const PixelBuffer &copy);
  void operator = (const PixelBuffer &copy);

  // Some named constructors for common PixelBuffer types.
  INLINE static PixelBuffer rgb_buffer(int xsize, int ysize);
  INLINE static PixelBuffer rgba_buffer(int xsize, int ysize);
  INLINE static PixelBuffer depth_buffer(int xsize, int ysize);
  INLINE static PixelBuffer stencil_buffer(int xsize, int ysize);

  INLINE ~PixelBuffer(void);

  virtual void config( void );

  bool read(const Filename &name);
  bool write(const Filename &name) const;

  bool load( const PNMImage& pnmimage );
  bool store( PNMImage& pnmimage ) const;

  void copy(const PixelBuffer *pb);
  
  virtual bool copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr);
  virtual bool copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
                    const RenderBuffer &rb);

  INLINE void set_xsize(int size);
  INLINE void set_ysize(int size);
  INLINE void set_xorg(int org);
  INLINE void set_yorg(int org);
  INLINE void set_size(int x_org, int y_org, int x_size, int y_size);
  INLINE void set_num_components(int num_components);
  INLINE void set_component_width(int component_width);
  INLINE void set_format(Format format);
  INLINE void set_loaded();

  INLINE int get_xsize() const;
  INLINE int get_ysize() const;
  INLINE int get_xorg() const;
  INLINE int get_yorg() const;
  INLINE int get_border() const;
  INLINE int get_num_components() const;
  INLINE int get_component_width() const;
  INLINE Format get_format() const;
  INLINE Type get_image_type() const;

  INLINE void set_uchar_rgb_texel(const uchar color[3],
                  int x, int y, int width);

private:
  INLINE void store_unscaled_byte(int &index, int value);
  INLINE void store_unscaled_short(int &index, int value);
  INLINE void store_scaled_byte(int &index, int value, double scale);
  INLINE void store_scaled_short(int &index, int value, double scale);
  INLINE double get_unsigned_byte(int &index) const;
  INLINE double get_unsigned_short(int &index) const;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageBuffer::init_type();
    register_type(_type_handle, "PixelBuffer",
                  ImageBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

  ////////////////////////////////////////////////////////////////////

protected:
  int _xsize;
  int _ysize;

  // pixelbuffer "origin" represents upper left screen point at which
  // pixelbuffer should be drawn using draw_pixel_buffer
  int _xorg;
  int _yorg;
  int _border;
  int _num_components;
  int _component_width;
  Format _format;
  Type _type;

  bool _loaded;

public:
  // This is public to allow direct manipulation of the image data.
  // Know what you are doing!
  PTA_uchar _image;
};

#include "pixelBuffer.I"

#endif

// Filename: pixelBuffer.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "imageBuffer.h"

#include <pnmImage.h>
#include <graphicsStateGuardianBase.h>
#include <pta_uchar.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

class RenderBuffer;

////////////////////////////////////////////////////////////////////
//       Class : PixelBuffer
// Description :
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
    F_rgb5,    // specifically, 5 bits per R,G,B channel
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
    F_luminance_alpha
  };

  INLINE PixelBuffer(void);
  INLINE PixelBuffer(int xsize, int ysize, int components, 
		     int component_width, Type type, Format format);
  INLINE PixelBuffer(const PixelBuffer &copy);
  INLINE void operator = (const PixelBuffer &copy);

  // Some named constructors for common PixelBuffer types.
  INLINE static PixelBuffer rgb_buffer(int xsize, int ysize);
  INLINE static PixelBuffer rgba_buffer(int xsize, int ysize);
  INLINE static PixelBuffer depth_buffer(int xsize, int ysize);
  INLINE static PixelBuffer stencil_buffer(int xsize, int ysize);

  INLINE ~PixelBuffer(void);

  virtual void config( void );

  virtual bool read( const string& name );
  virtual bool write( const string& name = "" ) const;

  bool load( const PNMImage& pnmimage );
  bool store( PNMImage& pnmimage ) const;

  void copy(const PixelBuffer *pb);

  virtual void copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr);
  virtual void copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
			const RenderBuffer &rb);
  virtual void draw(GraphicsStateGuardianBase *gsg);
  virtual void draw(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr);
  virtual void draw(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
			const RenderBuffer &rb);

  INLINE void set_xsize(int size);
  INLINE void set_ysize(int size);
  INLINE void set_xorg(int org);
  INLINE void set_yorg(int org);
  INLINE void set_format(Format format);

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
  INLINE void store_unsigned_byte(int &index, double value);
  INLINE void store_unsigned_short(int &index, double value);
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
  int _xorg;
  int _yorg;
  int _border;
  int _components;
  int _component_width;
  Format _format;
  Type _type;

public:
  // This is public to allow direct manipulation of the image data.
  // Know what you are doing!
  PTA_uchar _image;
};

#include "pixelBuffer.I"

#endif

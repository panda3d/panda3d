// Filename: texture.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef TEXTURE_H
#define TEXTURE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "imageBuffer.h"
#include "pixelBuffer.h"
#include <graphicsStateGuardianBase.h>
#include <map>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : Texture
// Description : 2D texture class
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Texture : public ImageBuffer {
public:
  enum FilterType {
    // Mag Filter and Min Filter
    FT_nearest,
    FT_linear,
 
    // Min Filter Only
    FT_nearest_mipmap_nearest,
    FT_linear_mipmap_nearest,
    FT_nearest_mipmap_linear,
    FT_linear_mipmap_linear,
  };
 
  enum WrapMode {
    WM_clamp,
    WM_repeat,
  };
	
  Texture();
  ~Texture();

  virtual bool read( const string& name );
  virtual bool write( const string& name = "" ) const;

  bool load( const PNMImage& pnmimage );
  bool store( PNMImage& pnmimage ) const;

  TextureContext *prepare(GraphicsStateGuardianBase *gsg);
  void unprepare();
  void unprepare(GraphicsStateGuardianBase *gsg);
  void clear_gsg(GraphicsStateGuardianBase *gsg);

  INLINE void apply( GraphicsStateGuardianBase *gsg );

  virtual void copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr);
  virtual void copy(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
			const RenderBuffer &rb);
  virtual void draw(GraphicsStateGuardianBase *gsg);
  virtual void draw(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr);
  virtual void draw(GraphicsStateGuardianBase *gsg, const DisplayRegion *dr,
			const RenderBuffer &rb);

  INLINE bool has_ram_image(void) const {
    return !_pbuffer->_image.empty();
  }
 
  void set_wrapu( WrapMode wrap );
  void set_wrapv( WrapMode wrap );
  void set_minfilter( FilterType filter );
  void set_magfilter( FilterType filter );

  INLINE int get_level() const;
  INLINE WrapMode get_wrapu() const;
  INLINE WrapMode get_wrapv() const;
  INLINE FilterType get_minfilter() const;
  INLINE FilterType get_magfilter() const;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWriteable *make_Texture(const FactoryParams &params);

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

  ////////////////////////////////////////////////////////////////////
	
protected:

  int _level;
  WrapMode _wrapu;
  WrapMode _wrapv;
  FilterType _minfilter;
  FilterType _magfilter;
  FilterType _magfiltercolor;
  FilterType _magfilteralpha;

  // A Texture keeps a list (actually, a map) of all the GSG's that it
  // has been prepared into.  Each GSG conversely keeps a list (a set)
  // of all the Texture's that have been prepared there.  When either
  // destructs, it removes itself from the other's list.
  typedef map<GraphicsStateGuardianBase *, TextureContext *> Contexts;
  Contexts _contexts;


  // These are public to allow direct manipulation of the underlying
  // pixel buffer when needed.  Know what you are doing!
public:
  PT(PixelBuffer) _pbuffer;

  // If you request a region from the framebuffer that is not a power of 2,
  // we need to grab a larger region that is a power of 2 that contains the
  // requested region and set the pixel buffer size accordingly.  We store
  // the size you requested in the members below.
  bool _has_requested_size;
  int _requested_w;
  int _requested_h;
};

#include "texture.I"

#endif


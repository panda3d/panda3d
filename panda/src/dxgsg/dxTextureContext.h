// Filename: dxTextureContext.h
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef DXTEXTURECONTEXT_H
#define DXTEXTURECONTEXT_H

#include <pandabase.h>

// Must include windows.h before gl.h on NT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <texture.h>
#include <textureContext.h>

#include <d3d.h>

#define MAX_DX_TEXPIXFMTS 20    // should be enough for any card

////////////////////////////////////////////////////////////////////
//   Class : DXTextureContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXTextureContext : public TextureContext {
public:
  DXTextureContext(Texture *tex);
  ~DXTextureContext();

  LPDIRECTDRAWSURFACE7  _surface;
  Texture *_tex;            // ptr to parent, primarily for access to namestr

  LPDIRECTDRAWSURFACE7 CreateTexture(LPDIRECT3DDEVICE7 pd3dDevice, int cNumTexPixFmts, LPDDPIXELFORMAT pTexPixFmts);
       
  void DeleteTexture();

  bool _bHasMipMaps;
  DWORD _PixBufConversionType;  // enum ConversionType
  HRESULT FillDDSurfTexturePixels(void);

protected:
    unsigned int get_bits_per_pixel(PixelBuffer::Format format, int *alphbits);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextureContext::init_type();
    register_type(_type_handle, "DXTextureContext",
          TextureContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

  friend class DXGraphicsStateGuardian;
};


#endif


// Filename: dxSavedFrameBuffer.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef DXSAVEDFRAMEBUFFER_H
#define DXSAVEDFRAMEBUFFER_H

#include <pandabase.h>

#include <savedFrameBuffer.h>
#include <texture.h>
#include <textureContext.h>
#include <pixelBuffer.h>


////////////////////////////////////////////////////////////////////
//   Class : DXSavedFrameBuffer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXSavedFrameBuffer : public SavedFrameBuffer {
public:
  INLINE DXSavedFrameBuffer(const RenderBuffer &buffer,
                CPT(DisplayRegion) dr);
  INLINE ~DXSavedFrameBuffer();

  PT(Texture) _back_rgba;
  PT(PixelBuffer) _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedFrameBuffer::init_type();
    register_type(_type_handle, "DXSavedFrameBuffer",
          SavedFrameBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "dxSavedFrameBuffer.I"

#endif


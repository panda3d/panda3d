// Filename: textureContext.h
// Created by:  drose (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTURECONTEXT_H
#define TEXTURECONTEXT_H

#include <pandabase.h>

#include <typedObject.h>

class Texture;

////////////////////////////////////////////////////////////////////
//       Class : TextureContext
// Description : This is a special class object that holds all the
//               information returned by a particular GSG to indicate
//               the texture's internal context identifier.
//
//               Textures typically have an immediate-mode and a
//               retained-mode operation.  When using textures in
//               retained-mode (in response to
//               Texture::prepare_texture()), the GSG will create some
//               internal handle for the texture and store it here.
//               The texture stores all of these handles internally.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureContext : public TypedObject {
public:
  INLINE TextureContext(Texture *tex);

  virtual size_t estimate_texture_memory();

  // This cannot be a PT(Texture), because the texture and the GSG
  // both own their TextureContexts!  That would create a circular
  // reference count.
  Texture *_texture;
 
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "TextureContext",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "textureContext.I"

#endif


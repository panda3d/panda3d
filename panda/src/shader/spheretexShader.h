// Filename: spheretexShader.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef SPHERETEXSHADER_H
#define SPHERETEXSHADER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "shader.h"
#include <texture.h>
#include <colorBlendTransition.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : SpheretexShader
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_SHADER SpheretexShader : public Shader
{
  public:
	
    SpheretexShader(Texture* texture = NULL);
    ~SpheretexShader() { }

    virtual void config(void);
    virtual void apply(Node *node, const AllAttributesWrapper &init_state,
		       const AllTransitionsWrapper &net_trans,
		       GraphicsStateGuardian *gsg);

    INLINE void set_texture(Texture* texture) {
      _texture = texture;
      make_dirty();
    }
    INLINE Texture* get_texture(void) { return _texture; }
    INLINE void set_blend_mode(ColorBlendProperty::Mode mode) {
      _blend_mode = mode;
    }

  protected:
	
    PT(Texture)			_texture;
    ColorBlendProperty::Mode	_blend_mode;

  public:

    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      Shader::init_type();
      register_type(_type_handle, "SpheretexShader",
                          Shader::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
  private:

    static TypeHandle _type_handle;
};

#endif

// Filename: directionalLight.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsStateGuardian.h>
#include <luse.h>
#include <map>
#include <namedNode.h>
#include "light.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : DirectionalLight
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectionalLight : public Light, public NamedNode
{
  public:

    DirectionalLight( const string& name = "" );
    ~DirectionalLight( void ) { }

    virtual void apply( GraphicsStateGuardian* gsg ) {
      gsg->apply_light( this );
    }

    INLINE Colorf get_specular( void ) const;
    INLINE void set_specular( const Colorf& color );

    virtual void output( ostream &out ) const;
    virtual void write( ostream &out, int indent_level = 0 ) const;

  protected:
	
    Colorf                                  _specular;

  public:

    static TypeHandle get_class_type( void ) {
      return _type_handle;
    }
    static void init_type( void ) {
      Light::init_type();
      NamedNode::init_type();
      register_type( _type_handle, "DirectionalLight",
                        Light::get_class_type(),
			NamedNode::get_class_type() );
    }
    virtual TypeHandle get_type( void ) const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
    virtual TypeHandle get_light_type( void ) const {
      return get_class_type();
    }

  private:

    static TypeHandle                       _type_handle;
};

INLINE ostream &operator << (ostream &out, const DirectionalLight &light) {
  light.output(out);
  return out;
}

#include "directionalLight.I"

#endif

// Filename: material.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef MATERIAL_H
#define MATERIAL_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsStateGuardianBase.h>
#include <typedReferenceCount.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Material
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Material : public TypedReferenceCount
{
    public:

	Material( void );
	~Material( void );

  	void apply( GraphicsStateGuardianBase *gsg ) {
	    gsg->apply_material( this );
	}

	INLINE Colorf get_ambient( void ) const;
	INLINE void set_ambient( const Colorf& color );
	INLINE Colorf get_diffuse( void ) const;
	INLINE void set_diffuse( const Colorf& color );
	INLINE Colorf get_specular( void ) const;
	INLINE void set_specular( const Colorf& color );
	INLINE float get_shininess( void ) const;
	INLINE void set_shininess( float shininess );
	INLINE Colorf get_emission( void ) const;
	INLINE void set_emission( const Colorf& color );

	INLINE bool get_local( void ) const;
	INLINE void set_local( bool local );
	INLINE bool get_twoside( void ) const;
	INLINE void set_twoside( bool twoside );

	void output( ostream &out ) const;
	void write( ostream &out, int indent = 0 ) const;

    protected:

	Colorf				_ambient;
	Colorf				_diffuse;
	Colorf				_specular;
	float				_shininess;
	Colorf				_emission;

	bool				_local;
	bool				_twoside;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "Material",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const Material &m) {
  m.output(out);
  return out;
}

#include "material.I"

#endif

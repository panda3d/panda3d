// Filename: light.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef LIGHT_H
#define LIGHT_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <typedObject.h>
#include <graphicsStateGuardian.h>
#include <referenceCount.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Light
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Light : virtual public ReferenceCount
{
    PUBLISHED:

        Light( void ) { _color.set(0.0, 0.0, 0.0, 1.0); }
        virtual ~Light( void ) { }

        INLINE Colorf get_color(void) const { return _color; }
        INLINE void set_color(const Colorf& color) { _color = color; }

        virtual void output(ostream &out) const=0;
        virtual void write(ostream &out, int indent_level = 0) const=0;

    public:
        virtual void apply( GraphicsStateGuardian* gsg ) = 0;

    protected:

        Colorf                  _color;

    public:

        static TypeHandle get_class_type( void ) {
            return _type_handle;
        }
        static void init_type( void ) {
            ReferenceCount::init_type();
            register_type( _type_handle, "Light",
                        ReferenceCount::get_class_type() );
        }
        virtual TypeHandle get_light_type( void ) const {
            return get_class_type();
        }

    private:

        static TypeHandle                       _type_handle;
};

INLINE ostream &operator << (ostream &out, const Light &light) {
  light.output(out);
  return out;
}

#endif

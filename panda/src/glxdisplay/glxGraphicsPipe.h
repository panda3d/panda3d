// Filename: glxGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef GLXGRAPHICSPIPE_H
#define GLXGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <interactiveGraphicsPipe.h>
#include "glxGraphicsWindow.h"
#include <X11/Xlib.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class Xclass;

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class glxGraphicsPipe : public InteractiveGraphicsPipe
{
    public:
	
    	glxGraphicsPipe( const PipeSpecifier& );

        virtual TypeHandle get_window_type() const;

        glxGraphicsWindow* find_window(Window win);

    public:

        static GraphicsPipe* make_glxGraphicsPipe(const FactoryParams &params);

        static TypeHandle get_class_type(void);
        static void init_type(void);
        virtual TypeHandle get_type(void) const;
        virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

	INLINE Display* get_display(void) { return _display; }
	INLINE int get_screen(void) { return _screen; }
	INLINE Window get_root(void) { return _root; }

    private:

        static TypeHandle _type_handle;

	Display*			_display;
	int				_screen;
	Window				_root;
	int				_width;
	int				_height;

    protected:

        glxGraphicsPipe( void );
        glxGraphicsPipe( const glxGraphicsPipe& );
        glxGraphicsPipe& operator=(const glxGraphicsPipe&);
};

#endif

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

#include "glxGraphicsWindow.h"
#include "glxDisplay.h"

#include <interactiveGraphicsPipe.h>

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class glxGraphicsPipe : public InteractiveGraphicsPipe, public glxDisplay
{
    public:

        glxGraphicsPipe( const PipeSpecifier& );

        virtual TypeHandle get_window_type() const;

        virtual glxDisplay *get_glx_display();

    public:

        static GraphicsPipe* make_glxGraphicsPipe(const FactoryParams &params);

        static TypeHandle get_class_type(void);
        static void init_type(void);
        virtual TypeHandle get_type(void) const;
        virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

    private:

        static TypeHandle _type_handle;
};

#endif

// Filename: sgiHardwareChannel.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "sgiHardwareChannel.h"
#include "sgiGraphicsPipe.h"
#include <GL/glx.h>
#include <X11/extensions/XSGIvc.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle sgiHardwareChannel::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Constructor 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
sgiHardwareChannel::sgiHardwareChannel( GraphicsWindow* window, int id ) :
	HardwareChannel( window )
{
    _id = id;
    const GraphicsPipe* pipe = window->get_pipe();
    const sgiGraphicsPipe* sgipipe;
    if ( pipe->get_class_type() == sgiGraphicsPipe::get_class_type() )
    	sgipipe = (sgiGraphicsPipe *)pipe;
    else
    {
	cerr << "sgiHardwareChannel::Constructor() - window does not have "
		<< "an sgiGraphicsPipe!!!" << endl;
	return;
    }

    XSGIvcChannelInfo* channel_info = (XSGIvcChannelInfo *)_channel_info;

    XSGIvcQueryChannelInfo( (Display *)sgipipe->get_display(), 
	sgipipe->get_screen(), _id, &channel_info );
    if ( channel_info->active ) {
      int screeny = DisplayHeight( (Display *)sgipipe->get_display(), 
				   sgipipe->get_screen() );
      _xsize = channel_info->source.width;
      _ysize = channel_info->source.height;
      _xorg = channel_info->source.x;
      _yorg = screeny - ( channel_info->source.y + _ysize );
      set_active(true);
    } else {
      set_active(false);
    }
}

// Filename: sgiGraphicsPipe.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "sgiGraphicsPipe.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <X11/extensions/XSGIvc.h>
#include "sgiHardwareChannel.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle sgiGraphicsPipe::_type_handle;

sgiGraphicsPipe::sgiGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec) {
  _num_channels = -1;
}

sgiGraphicsPipe::~sgiGraphicsPipe(void) {
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_hw_channels
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int sgiGraphicsPipe::get_num_hw_channels( void )
{
    if ( _num_channels == -1 )
    {
        Display* display = (Display *)_display;
        // Is there a better way to get the display??? (I hope so)
        display = glXGetCurrentDisplayEXT(); 

        // For now, screen is the same as display
        _screen = DefaultScreen( display );

        XSGIvcScreenInfo server;
        XSGIvcQueryVideoScreenInfo( display, _screen, &server );

        // Not all of these will be active necessarily
        _num_channels = server.numChannels; 
    }

    return _num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: get_hw_channel
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
HardwareChannel *sgiGraphicsPipe::
get_hw_channel( GraphicsWindow* window, int index ) {
    if ( _num_channels == -1 )
        get_num_hw_channels();

    if ( index >= _num_channels )
    {
        cerr << "sgiGraphicsPipe::get_hw_channel() - invalid index: " 
                << index << endl;
        return NULL;
    }

    Channels::iterator i;
    i = _hw_chans.find( index ); 
    if (i != _hw_chans.end()) {
      return (*i).second.p();
    }

    sgiHardwareChannel* hw_chan;
    hw_chan = new sgiHardwareChannel( window, index );
    _hw_chans[index] = hw_chan;

    return hw_chan;
}

TypeHandle sgiGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void sgiGraphicsPipe::init_type(void) {
  InteractiveGraphicsPipe::init_type();
  register_type(_type_handle, "sgiGraphicsPipe",
                InteractiveGraphicsPipe::get_class_type());
}

TypeHandle sgiGraphicsPipe::get_type(void) const {
  return get_class_type();
}

sgiGraphicsPipe::sgiGraphicsPipe(void) {
  cerr << "sgiGraphicsPipes should not be created with default constructor"
       << endl;
}

sgiGraphicsPipe::sgiGraphicsPipe(const sgiGraphicsPipe&) {
  cerr << "sgiGraphicsPipes should not be copied" << endl;
}

sgiGraphicsPipe& sgiGraphicsPipe::operator=(const sgiGraphicsPipe&) {
  cerr << "sgiGraphicsPipes should not be assigned" << endl;
  return *this;
}

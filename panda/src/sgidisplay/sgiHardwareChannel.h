// Filename: graphicsChannel.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef SGIHARDWARECHANNEL_H
#define SGIHARDWARECHANNEL_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <hardwareChannel.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : sgiHardwareChannel
// Description :
////////////////////////////////////////////////////////////////////
class sgiHardwareChannel : public HardwareChannel
{
    public:

    	sgiHardwareChannel( GraphicsWindow* window, int id );

    protected:

	void 			*_channel_info;

    public:

        static TypeHandle get_class_type() {
            return _type_handle;
        }
        static void init_type() {
            HardwareChannel::init_type();
            register_type(_type_handle, "sgiHardwareChannel",
                          HardwareChannel::get_class_type());
        }
        virtual TypeHandle get_type() const {
            return get_class_type();
        }
        virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

    private:

        static TypeHandle _type_handle;
};

#endif

// Filename: iPhoneGraphicsStateGuardian.cxx
// Created by:  drose (08Apr09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "iPhoneGraphicsStateGuardian.h"
#include "string_utils.h"
#include "config_iphonedisplay.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "textureAttrib.h"
#include "pnmImage.h"
#include "glesgsg.h"

TypeHandle IPhoneGraphicsStateGuardian::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: IPhoneGraphicsStateGuardian::get_extension_func
//       Access: Public, Virtual
//  Description: Returns the pointer to the GL extension function with
//               the indicated name.  It is the responsibility of the
//               caller to ensure that the required extension is
//               defined in the OpenGL runtime prior to calling this;
//               it is an error to call this for a function that is
//               not defined.
////////////////////////////////////////////////////////////////////
void *IPhoneGraphicsStateGuardian::
get_extension_func(const char *prefix, const char *name) {      
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: IPhoneGraphicsStateGuardian::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IPhoneGraphicsStateGuardian::
IPhoneGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                         IPhoneGraphicsStateGuardian *share_with) :
  GLESGraphicsStateGuardian(engine, pipe),
  _share_with(share_with)
{
  _shared_buffer = 1011;
}

////////////////////////////////////////////////////////////////////
//     Function: IPhoneGraphicsStateGuardian::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IPhoneGraphicsStateGuardian::
~IPhoneGraphicsStateGuardian() {
}

////////////////////////////////////////////////////////////////////
//     Function: IPhoneGraphicsStateGuardian::reset
//       Access: Public, Virtual
//  Description: Resets all internal state as if the gsg were newly
//               created.
////////////////////////////////////////////////////////////////////
void IPhoneGraphicsStateGuardian::
reset() {
  GLESGraphicsStateGuardian::reset();
}

////////////////////////////////////////////////////////////////////
//     Function: IPhoneGraphicsStateGuardian::describe_pixel_format
//       Access: Private
//  Description: Fills in the fb_props member with the appropriate
//               values according to the chosen pixel format.
////////////////////////////////////////////////////////////////////
void IPhoneGraphicsStateGuardian::
describe_pixel_format(FrameBufferProperties &fb_props) {
}

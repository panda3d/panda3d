// Filename: savedFrameBuffer.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SAVEDFRAMEBUFFER_H
#define SAVEDFRAMEBUFFER_H

#include "pandabase.h"

#include "renderBuffer.h"
#include "displayRegion.h"

#include "typedReferenceCount.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : SavedFrameBuffer
// Description : Occasionally we need to save the contents of the
//               frame buffer for restoring later, particularly when
//               we are doing multipass effects.  The precise form in
//               which the frame buffer is optimally saved may vary
//               from one platform to another; hence, we have the
//               SavedFrameBuffer class, which is a placeholder
//               structure to store the frame buffer in whichever way
//               a particular GSG would prefer to do it.
//
//               Each specific GSG will also derive a new kind of
//               SavedFrameBuffer object that it will use to store its
//               frame buffer contents meaningfully.
//
//               This class is not meant to be used directly; it is
//               used within the FrameBufferStack class to support
//               GraphicsStateGuardian::push_frame_buffer() and
//               pop_frame_buffer().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SavedFrameBuffer : public TypedReferenceCount {
public:
  INLINE SavedFrameBuffer(const RenderBuffer &buffer,
                          CPT(DisplayRegion) dr);

  RenderBuffer _buffer;
  CPT(DisplayRegion) _display_region;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "SavedFrameBuffer",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "savedFrameBuffer.I"

#endif


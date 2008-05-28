// Filename: graphicsDevice.h
// Created by:  masad (21Jul03)
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

#ifndef GRAPHICSDEVICE_H
#define GRAPHICSDEVICE_H

#include "pandabase.h"

#include "typedReferenceCount.h"

class GraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsDevice
// Description : An abstract device object that is part of Graphics
//               Pipe. This device is set to NULL for OpenGL. But 
//               DirectX uses it to take control of multiple windows
//               under single device or multiple devices (i.e. more
//               than one adapters in the machine).   
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY GraphicsDevice : public TypedReferenceCount {
public:
  GraphicsDevice(GraphicsPipe *pipe);

private:
  GraphicsDevice(const GraphicsDevice &copy);
  void operator = (const GraphicsDevice &copy);

PUBLISHED:
  virtual ~GraphicsDevice();

  INLINE GraphicsPipe *get_pipe() const;

protected:
  GraphicsPipe *_pipe;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsDevice",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsEngine;
};

#include "graphicsDevice.I"

#endif /* GRAPHICSDEVICE_H */

// Filename: sgiGraphicsPipe.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef SGIGRAPHICSPIPE_H
#define SGIGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "sgiHardwareChannel.h"

#include <interactiveGraphicsPipe.h>
#include <string>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : sgiGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class sgiGraphicsPipe : public InteractiveGraphicsPipe {
public:

  sgiGraphicsPipe(const PipeSpecifier&);
  virtual ~sgiGraphicsPipe() = 0;

  INLINE void* get_display() const { return _display; }
  INLINE int get_screen() const { return _screen; }

protected:

  typedef map<int,  PT(sgiHardwareChannel) > Channels;
  Channels _hw_chans;

  virtual int get_num_hw_channels();
  virtual HardwareChannel *get_hw_channel(GraphicsWindow* window,
                                          int index);

public:

  static TypeHandle get_class_type();
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

protected:

  void *_display;
  int _screen;
  int _num_channels;

private:

  static TypeHandle _type_handle;

protected:

  sgiGraphicsPipe();
  sgiGraphicsPipe(const sgiGraphicsPipe&);
  sgiGraphicsPipe& operator=(const sgiGraphicsPipe&);
};

#endif

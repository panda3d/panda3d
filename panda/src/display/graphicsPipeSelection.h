// Filename: graphicsPipeSelection.h
// Created by:  drose (15Aug02)
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

#ifndef GRAPHICSPIPESELECTION_H
#define GRAPHICSPIPESELECTION_H

#include "pandabase.h"

#include "graphicsPipe.h"
#include "pointerTo.h"
#include "typeHandle.h"
#include "pmutex.h"
#include "vector_string.h"

class HardwareChannel;
class GraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsPipeSelection
// Description : This maintains a list of GraphicsPipes by type that
//               are available for creation.  Normally there is one
//               default interactive GraphicsPipe, and possibly other
//               types available as well.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsPipeSelection {
protected:
  GraphicsPipeSelection();
  ~GraphicsPipeSelection();

PUBLISHED:
  int get_num_pipe_types() const;
  TypeHandle get_pipe_type(int n) const;
  void print_pipe_types() const;

  PT(GraphicsPipe) make_pipe(TypeHandle type);
  PT(GraphicsPipe) make_default_pipe();

  INLINE int get_num_aux_modules() const;
  void load_aux_modules();

  INLINE static GraphicsPipeSelection *get_global_ptr();

public:
  typedef PT(GraphicsPipe) PipeConstructorFunc();
  bool add_pipe_type(TypeHandle type, PipeConstructorFunc *func);

private:
  INLINE void load_default_module() const;
  void do_load_default_module();
  void load_named_module(const string &name);

  class PipeType {
  public:
    INLINE PipeType(TypeHandle type, PipeConstructorFunc *constructor);
    TypeHandle _type;
    PipeConstructorFunc *_constructor;
  };
  typedef pvector<PipeType> PipeTypes;
  PipeTypes _pipe_types;
  Mutex _lock;

  typedef vector_string DisplayModules;
  DisplayModules _display_modules;
  string _default_display_module;
  string _default_pipe_name;
  bool _default_module_loaded;

  static GraphicsPipeSelection *_global_ptr;
};  

#include "graphicsPipeSelection.I"

#endif


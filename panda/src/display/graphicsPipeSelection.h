/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipeSelection.h
 * @author drose
 * @date 2002-08-15
 */

#ifndef GRAPHICSPIPESELECTION_H
#define GRAPHICSPIPESELECTION_H

#include "pandabase.h"

#include "graphicsPipe.h"
#include "pointerTo.h"
#include "typeHandle.h"
#include "lightMutex.h"
#include "vector_string.h"

class GraphicsWindow;

/**
 * This maintains a list of GraphicsPipes by type that are available for
 * creation.  Normally there is one default interactive GraphicsPipe, and
 * possibly other types available as well.
 */
class EXPCL_PANDA_DISPLAY GraphicsPipeSelection {
protected:
  GraphicsPipeSelection();
  ~GraphicsPipeSelection();

PUBLISHED:
  int get_num_pipe_types() const;
  TypeHandle get_pipe_type(int n) const;
  MAKE_SEQ(get_pipe_types, get_num_pipe_types, get_pipe_type);
  MAKE_SEQ_PROPERTY(pipe_types, get_num_pipe_types, get_pipe_type);
  void print_pipe_types() const;

  PT(GraphicsPipe) make_pipe(const std::string &type_name,
                             const std::string &module_name = std::string());
  PT(GraphicsPipe) make_pipe(TypeHandle type);
  PT(GraphicsPipe) make_module_pipe(const std::string &module_name);
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
  TypeHandle load_named_module(const std::string &name);

  class LoadedModule {
  public:
    std::string _module_name;
    void *_module_handle;
    TypeHandle _default_pipe_type;
  };
  typedef pmap<std::string, LoadedModule> LoadedModules;
  LoadedModules _loaded_modules;
  LightMutex _loaded_modules_lock;

  class PipeType {
  public:
    INLINE PipeType(TypeHandle type, PipeConstructorFunc *constructor);
    TypeHandle _type;
    PipeConstructorFunc *_constructor;
  };
  typedef pvector<PipeType> PipeTypes;
  PipeTypes _pipe_types;
  LightMutex _lock;

  typedef vector_string DisplayModules;
  DisplayModules _display_modules;
  std::string _default_display_module;
  std::string _default_pipe_name;
  bool _default_module_loaded;

  static GraphicsPipeSelection *_global_ptr;
};

#include "graphicsPipeSelection.I"

#endif

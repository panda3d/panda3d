/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketRegion.h
 * @author rdb
 * @date 2011-11-30
 */

#ifndef ROCKETREGION_H
#define ROCKETREGION_H

#include "config_rocket.h"
#include "displayRegion.h"
#include "rocketRenderInterface.h"
#include "rocketInputHandler.h"

class OrthographicLens;

/**
 * Represents a region in a window or buffer where the libRocket UI will be
 * rendered to.
 */
class EXPCL_ROCKET RocketRegion : public DisplayRegion {
protected:
  RocketRegion(GraphicsOutput *window, const LVecBase4 &dimensions,
               const std::string &context_name);

  virtual void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
                       GraphicsStateGuardian *gsg, Thread *current_thread);

PUBLISHED:
  virtual ~RocketRegion();

  INLINE static RocketRegion* make(const std::string &context_name,
                                   GraphicsOutput *window);
  INLINE static RocketRegion* make(const std::string &context_name,
                                   GraphicsOutput *window,
                                   const LVecBase4 &dimensions);
#ifndef CPPPARSER
  INLINE Rocket::Core::Context* get_context() const;
#endif
#ifdef HAVE_ROCKET_PYTHON
  EXTENSION(PyObject *get_context() const);
  MAKE_PROPERTY(context, get_context);
#endif

  INLINE void set_input_handler(RocketInputHandler *handler);
  INLINE RocketInputHandler *get_input_handler() const;
  MAKE_PROPERTY(input_handler, get_input_handler, set_input_handler);

  bool init_debugger();
  void set_debugger_visible(bool visible);
  bool is_debugger_visible() const;
  MAKE_PROPERTY(debugger_visible, is_debugger_visible, set_debugger_visible);

private:
  RocketRenderInterface _interface;
  Rocket::Core::Context* _context;
  PT(OrthographicLens) _lens;
  PT(RocketInputHandler) _input_handler;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DisplayRegion::init_type();
    register_type(_type_handle, "RocketRegion",
                  DisplayRegion::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "rocketRegion.I"

#endif /* ROCKETREGION_H */

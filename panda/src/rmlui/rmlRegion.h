/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlRegion.h
 * @author rdb
 * @date 2011-11-30
 */

#ifndef RML_REGION_H
#define RML_REGION_H

#include "config_rmlui.h"
#include "displayRegion.h"
#include "rmlInputHandler.h"
#include "rmlContext.h"
#include "graphicsOutput.h"
#include "callbackObject.h"

#ifndef CPPPARSER
#include "rmlRenderInterface.h"
#endif

class OrthographicLens;

namespace Rml {
  class Context;
}

/**
 * A DisplayRegion that hosts an RmlUi context, rendering the UI into the
 * associated window or buffer region.
 *
 * Usage:
 *   region = RmlRegion.make("hud", window)
 *   doc = region.load_document("ui/hud.rml")
 *   doc.Show()
 *   # wire up input:
 *   handler = RmlInputHandler()
 *   mouseWatcher.attach_new_node(handler)
 *   region.set_input_handler(handler)
 */
class EXPCL_PANDARMLUI RmlRegion : public DisplayRegion {
protected:
  RmlRegion(GraphicsOutput *window, const LVecBase4 &dimensions,
            const std::string &context_name);

  virtual void do_cull(CullHandler *cull_handler, SceneSetup *scene_setup,
                       GraphicsStateGuardian *gsg, Thread *current_thread);

  // Performs the actual RmlUi render against the given cull handler (which must
  // draw immediately, i.e. a DrawCullHandler with the GSG frame open).  Shared
  // by the cull-and-draw-together path (called from do_cull) and the deferred
  // draw-callback path (called from the draw thread with the frame open).
  void render_now(CullHandler *cull_handler, SceneSetup *scene_setup,
                  GraphicsStateGuardian *gsg, Thread *current_thread);

PUBLISHED:
  virtual ~RmlRegion();

  INLINE static RmlRegion *make(const std::string &context_name,
                                GraphicsOutput *window);
  INLINE static RmlRegion *make(const std::string &context_name,
                                GraphicsOutput *window,
                                const LVecBase4 &dimensions);

  bool is_valid() const { return _rml_context != nullptr; }

  PT(RmlContext) get_context() const;
  MAKE_PROPERTY(context, get_context);

  INLINE void set_input_handler(RmlInputHandler *handler);
  INLINE RmlInputHandler *get_input_handler() const;
  MAKE_PROPERTY(input_handler, get_input_handler, set_input_handler);

  bool init_debugger();
  void set_debugger_visible(bool visible);
  bool is_debugger_visible() const;
  MAKE_PROPERTY(debugger_visible, is_debugger_visible, set_debugger_visible);

private:
  // Draw-phase callback.  In a sorted (cull/draw) threading model do_cull runs
  // in the cull phase with no open GSG frame, so RmlUi's layer compositing
  // (which needs an open command buffer to switch render targets) cannot run
  // there.  Instead do_cull defers, and this callback runs render_now() during
  // the draw phase, where the window frame is open.
  class DrawCallback : public CallbackObject {
  public:
    DrawCallback(RmlRegion *region) : _region(region) {}
    virtual void do_callback(CallbackData *cbdata);
    RmlRegion *_region;
  };

#ifndef CPPPARSER
  RmlRenderInterface _interface;
  Rml::Context *_context = nullptr;
#endif

  PT(RmlContext) _rml_context;
  PT(OrthographicLens) _lens;
  PT(RmlInputHandler) _input_handler;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DisplayRegion::init_type();
    register_type(_type_handle, "RmlRegion",
                  DisplayRegion::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
  static TypeHandle _type_handle;
};

#include "rmlRegion.I"

#endif

/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsThreadingModel.I
 * @author drose
 * @date 2003-01-27
 */

/**
 *
 */
INLINE GraphicsThreadingModel::
GraphicsThreadingModel(const GraphicsThreadingModel &copy) :
  _cull_name(copy._cull_name),
  _cull_stage(copy._cull_stage),
  _draw_name(copy._draw_name),
  _draw_stage(copy._draw_stage),
  _cull_sorting(copy._cull_sorting)
{
}

/**
 *
 */
INLINE void GraphicsThreadingModel::
operator = (const GraphicsThreadingModel &copy) {
  _cull_name = copy._cull_name;
  _cull_stage = copy._cull_stage;
  _draw_name = copy._draw_name;
  _draw_stage = copy._draw_stage;
  _cull_sorting = copy._cull_sorting;
}

/**
 * Returns the name of the thread that will handle culling in this model.
 */
INLINE const std::string &GraphicsThreadingModel::
get_cull_name() const {
  return _cull_name;
}

/**
 * Changes the name of the thread that will handle culling in this model.
 * This won't change any windows that were already created with this model;
 * this only has an effect on newly-opened windows.
 */
INLINE void GraphicsThreadingModel::
set_cull_name(const std::string &cull_name) {
  _cull_name = cull_name;
  update_stages();
}

/**
 * Returns the pipeline stage from which the cull thread should access data.
 * This will be 0 if the cull is run in the same thread as app, or 1 if it is
 * its own thread.
 */
INLINE int GraphicsThreadingModel::
get_cull_stage() const {
  return _cull_stage;
}

/**
 * Returns the name of the thread that will handle sending the actual graphics
 * primitives to the graphics API in this model.
 */
INLINE const std::string &GraphicsThreadingModel::
get_draw_name() const {
  return _draw_name;
}

/**
 * Changes the name of the thread that will handle drawing in this model.
 * This won't change any windows that were already created with this model;
 * this only has an effect on newly-opened windows.
 */
INLINE void GraphicsThreadingModel::
set_draw_name(const std::string &draw_name) {
  _draw_name = draw_name;
  update_stages();
}

/**
 * Returns the pipeline stage from which the draw thread should access data.
 * This will be the same value as get_cull_stage() if cull and draw are run in
 * the same thread, or one more than that value if draw should be in its own
 * thread.
 */
INLINE int GraphicsThreadingModel::
get_draw_stage() const {
  return _draw_stage;
}

/**
 * Returns true if the model involves a separate cull pass, or false if
 * culling happens implicitly, at the same time as draw.
 */
INLINE bool GraphicsThreadingModel::
get_cull_sorting() const {
  return _cull_sorting;

}

/**
 * Changes the flag that indicates whether the threading model involves a
 * separate cull pass.  This won't change any windows that were already
 * created with this model; this only has an effect on newly-opened windows.
 */
INLINE void GraphicsThreadingModel::
set_cull_sorting(bool cull_sorting) {
  _cull_sorting = cull_sorting;
  update_stages();
}

/**
 * Returns true if the threading model is a single-threaded model, or false if
 * it involves threads.
 */
INLINE bool GraphicsThreadingModel::
is_single_threaded() const {
  return _cull_name.empty() && _draw_name.empty();
}

/**
 * Returns true if the threading model is the default, cull-then-draw single-
 * threaded model, or false otherwise.
 */
INLINE bool GraphicsThreadingModel::
is_default() const {
  return is_single_threaded() && _cull_sorting;
}


/**
 *
 */
INLINE void GraphicsThreadingModel::
output(std::ostream &out) const {
  out << get_model();
}

INLINE std::ostream &
operator << (std::ostream &out, const GraphicsThreadingModel &threading_model) {
  threading_model.output(out);
  return out;
}

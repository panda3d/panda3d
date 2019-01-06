/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsThreadingModel.cxx
 * @author drose
 * @date 2003-01-27
 */

#include "graphicsThreadingModel.h"

using std::string;

/**
 * The threading model accepts a string representing the names of the two
 * threads that will process cull and draw for the given window, separated by
 * a slash.  The names are completely arbitrary and are used only to
 * differentiate threads.  The two names may be the same, meaning the same
 * thread, or each may be the empty string, which represents the previous
 * thread.
 *
 * Thus, for example, "cull/draw" indicates that the window will be culled in
 * a thread called "cull", and drawn in a separate thread called "draw".
 * "draw/draw" or simply "draw" indicates the window will be culled and drawn
 * in the same thread, "draw". On the other hand, "/draw" indicates the thread
 * will be culled in the main, or app thread, and drawn in a separate thread
 * named "draw".  The empty string, "" or "/", indicates the thread will be
 * culled and drawn in the main thread; that is to say, a single-process
 * model.
 *
 * Finally, if the threading model begins with a "-" character, then cull and
 * draw are run simultaneously, in the same thread, with no binning or state
 * sorting.  It simplifies the cull process but it forces the scene to render
 * in scene graph order; state sorting and alpha sorting is lost.
 */
GraphicsThreadingModel::
GraphicsThreadingModel(const string &model) {
  _cull_sorting = true;
  size_t start = 0;
  if (!model.empty() && model[0] == '-') {
    start = 1;
    _cull_sorting = false;
  }

  size_t slash = model.find('/', start);
  if (slash == string::npos) {
    _cull_name = model.substr(start);
  } else {
    _cull_name = model.substr(start, slash - start);
    _draw_name = model.substr(slash + 1);
  }

  update_stages();
}

/**
 * Returns the string that describes the threading model.  See the
 * constructor.
 */
string GraphicsThreadingModel::
get_model() const {
  if (get_cull_sorting()) {
    return get_cull_name() + "/" + get_draw_name();
  } else {
    return string("-") + get_cull_name();
  }
}

/**
 * Called internally to recompute _cull_stage and _draw_stage after either
 * name has been changed.
 */
void GraphicsThreadingModel::
update_stages() {
  if (_cull_name.empty()) {
    _cull_stage = 0;
  } else {
    _cull_stage = 1;
  }
  if (!_cull_sorting || _draw_name.empty()) {
    _draw_name = _cull_name;
  }

  if (_draw_name == _cull_name) {
    _draw_stage = _cull_stage;
  } else {
    _draw_stage = _cull_stage + 1;
  }
}

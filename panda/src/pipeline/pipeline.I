/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipeline.I
 * @author drose
 * @date 2002-02-21
 */

/**
 * Returns a pointer to the global render pipeline.
 */
INLINE Pipeline *Pipeline::
get_render_pipeline() {
  if (_render_pipeline == nullptr) {
    make_render_pipeline();
  }
  return _render_pipeline;
}

/**
 * Ensures that at least the indicated number of stages are in the pipeline.
 */
INLINE void Pipeline::
set_min_stages(int min_stages) {
  set_num_stages(std::max(min_stages, get_num_stages()));
}

/**
 * Returns the number of stages required for the pipeline.
 */
INLINE int Pipeline::
get_num_stages() const {
  return _num_stages;
}

#ifdef THREADED_PIPELINE
/**
 * Returns the number of PipelineCyclers in the universe that reference this
 * Pipeline object.
 */
INLINE int Pipeline::
get_num_cyclers() const {
  MutexHolder holder(_lock);
  return _num_cyclers;
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
/**
 * Returns the number of PipelineCyclers in the universe that reference this
 * Pipeline object and are currently marked "dirty"; that is, there is a
 * difference in pointer value between some of their stages.
 */
INLINE int Pipeline::
get_num_dirty_cyclers() const {
  MutexHolder holder(_lock);
  return _num_dirty_cyclers;
}
#endif  // THREADED_PIPELINE

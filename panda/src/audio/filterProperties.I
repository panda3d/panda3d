/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file filterProperties.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Removes all DSP postprocessing.
 */
INLINE void FilterProperties::
clear() {
  _config.clear();
}

/**
 *
 */
INLINE void FilterProperties::
add_filter(DSP *filter) {
  _config.push_back(filter);
}

/**
 * Intended for use by AudioManager and AudioSound implementations: allows
 * access to the config vector.
 */
INLINE const FilterProperties::ConfigVector &FilterProperties::
get_config() {
  return _config;
}

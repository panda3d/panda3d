/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullResult.I
 * @author drose
 * @date 2002-02-28
 */

/**
 *
 */
INLINE CullResult::
~CullResult() {
}

/**
 * Returns the CullBin associated with the indicated bin_index, or NULL if the
 * bin_index is invalid.  If there is the first time this bin_index has been
 * requested for this CullResult, creates a new CullBin object on the fly.
 */
INLINE CullBin *CullResult::
get_bin(int bin_index) {
  if (bin_index >= 0 && bin_index < (int)_bins.size() &&
      _bins[bin_index] != nullptr) {
    return _bins[bin_index];
  }
  return make_new_bin(bin_index);
}

/**
 * If the user configured flash-bin-binname, then update the object's state to
 * flash all the geometry in the bin.
 */
INLINE void CullResult::
check_flash_bin(CPT(RenderState) &state, CullBinManager *bin_manager, int bin_index) {
#ifndef NDEBUG
  if (bin_manager->get_bin_flash_active(bin_index)) {
    apply_flash_color(state, bin_manager->get_bin_flash_color(bin_index));
  }
#endif
}

/**
 * If the user configured show-transparency, then update the object's state to
 * flash the current geometry with the specified color.
 */
INLINE void CullResult::
check_flash_transparency(CPT(RenderState) &state, const LColor &color) {
#ifndef NDEBUG
  if (_show_transparency) {
    apply_flash_color(state, color);
  }
#endif
}

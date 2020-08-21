/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patchfile.I
 * @author darren, mike
 * @date 1997-01-09
 */

// #include "config_downloader.h"

/**
 * Returns a value in the range 0..1, representing the amount of progress
 * through the patchfile, during a session.
 */
INLINE PN_stdfloat Patchfile::
get_progress() const {
  if (!_initiated) {
    express_cat.warning()
      << "Patchfile::get_progress() - Patch has not been initiated" << std::endl;
    return 0.0f;
  }
  nassertr(_total_bytes_to_process > 0, 0.0f);
  return ((PN_stdfloat)_total_bytes_processed / (PN_stdfloat)_total_bytes_to_process);
}

/**
 * If this flag is set true, the Patchfile will make a special case for
 * patching Panda Multifiles, if detected, and attempt to patch them on a
 * subfile-by-subfile basis.  If this flag is false, the Patchfile will always
 * patch the file on a full-file basis.
 *
 * This has effect only when building patches; it is not used for applying
 * patches.
 */
INLINE void Patchfile::
set_allow_multifile(bool allow_multifile) {
  _allow_multifile = allow_multifile;
}

/**
 * See set_allow_multifile().
 */
INLINE bool Patchfile::
get_allow_multifile() {
  return _allow_multifile;
}

/**
 *
 */
INLINE void Patchfile::
set_footprint_length(int length) {
  nassertv(length > 0);
  _footprint_length = length;
}

/**
 *
 */
INLINE int Patchfile::
get_footprint_length() {
  return _footprint_length;
}

/**
 *
 */
INLINE void Patchfile::
reset_footprint_length() {
  _footprint_length = _DEFAULT_FOOTPRINT_LENGTH;
}

/**
 * Returns true if the MD5 hash for the source file is known.  (Some early
 * versions of the patch file did not store this information.)
 */
INLINE bool Patchfile::
has_source_hash() const {
  return (_version_number >= 1);
}

/**
 * Returns the MD5 hash for the source file.
 */
INLINE const HashVal &Patchfile::
get_source_hash() const {
  nassertr(has_source_hash(), _MD5_ofSource);
  return _MD5_ofSource;
}

/**
 * Returns the MD5 hash for the file after the patch has been applied.
 */
INLINE const HashVal &Patchfile::
get_result_hash() const {
  return _MD5_ofResult;
}

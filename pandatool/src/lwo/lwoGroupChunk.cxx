/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoGroupChunk.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoGroupChunk.h"
#include "lwoInputFile.h"

#include "pnotify.h"

TypeHandle LwoGroupChunk::_type_handle;

/**
 * Returns the number of child chunks of this group.
 */
int LwoGroupChunk::
get_num_chunks() const {
  return _chunks.size();
}

/**
 * Returns the nth child chunk of this group.
 */
IffChunk *LwoGroupChunk::
get_chunk(int n) const {
  nassertr(n >= 0 && n < (int)_chunks.size(), nullptr);
  return _chunks[n];
}

/**
 * Reads a sequence of child chunks, until byte stop_at has been been reached,
 * and stores them as the children.  Returns true if successful (and exactly
 * the correct number of bytes were read), or false otherwise.
 */
bool LwoGroupChunk::
read_chunks_iff(IffInputFile *in, size_t stop_at) {
  while (in->get_bytes_read() < stop_at && !in->is_eof()) {
    PT(IffChunk) chunk = in->get_chunk();
    if (chunk == nullptr) {
      return false;
    }
    _chunks.push_back(chunk);
  }

  return (in->get_bytes_read() == stop_at);
}

/**
 * Similar to read_chunks_iff(), but reads them as subchunks.
 */
bool LwoGroupChunk::
read_subchunks_iff(IffInputFile *in, size_t stop_at) {
  while (in->get_bytes_read() < stop_at && !in->is_eof()) {
    PT(IffChunk) chunk = in->get_subchunk(this);
    if (chunk == nullptr) {
      return false;
    }
    _chunks.push_back(chunk);
  }

  return (in->get_bytes_read() == stop_at);
}

/**
 * Formats the list of chunks for output to the user (primarily for
 * debugging), one per line.
 */
void LwoGroupChunk::
write_chunks(std::ostream &out, int indent_level) const {
  Chunks::const_iterator ci;
  for (ci = _chunks.begin(); ci != _chunks.end(); ++ci) {
    (*ci)->write(out, indent_level);
  }
}

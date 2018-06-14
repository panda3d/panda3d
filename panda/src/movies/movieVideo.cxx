/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieVideo.cxx
 * @author jyelon
 * @date 2007-07-02
 */

#include "movieVideo.h"
#include "movieVideoCursor.h"
#include "config_movies.h"
#include "movieTypeRegistry.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle MovieVideo::_type_handle;

/**
 * This constructor returns a null video stream --- a stream of plain blue and
 * white frames that last one second each.  To get more interesting video, you
 * need to construct a subclass of this class.
 */
MovieVideo::
MovieVideo(const std::string &name) :
  Namable(name)
{
}

/**
 *
 */
MovieVideo::
~MovieVideo() {
}

/**
 * Open this video, returning a MovieVideoCursor of the appropriate type.
 * Returns NULL on error.
 */
PT(MovieVideoCursor) MovieVideo::
open() {
  return nullptr;
}

/**
 * Obtains a MovieVideo that references a file.  Just calls
 * MovieTypeRegistry::make_video().
 */
PT(MovieVideo) MovieVideo::
get(const Filename &name) {
  MovieTypeRegistry *reg = MovieTypeRegistry::get_global_ptr();
  return reg->make_video(name);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MovieVideo::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);
  dg.add_string(_filename);

  // Now we record the raw movie data directly into the bam stream.  We always
  // do this, regardless of bam-texture-mode; we generally won't get to this
  // codepath if bam-texture-mode isn't rawdata anyway.

  SubfileInfo result;
  if (!_subfile_info.is_empty()) {
    dg.add_bool(true);
    manager->write_file_data(result, _subfile_info);
  } else if (!_filename.empty()) {
    dg.add_bool(true);
    manager->write_file_data(result, _filename);
  } else {
    dg.add_bool(false);
  }

  /* Not sure yet if this is a good idea.
  if (!result.is_empty()) {
    // If we've just copied the data to a local file, read it from there in
    // the future.
    _subfile_info = result;
  }
  */
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MovieVideo.
 */
void MovieVideo::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritableReferenceCount::fillin(scan, manager);
  _filename = scan.get_string();

  bool got_info = scan.get_bool();
  if (got_info) {
    manager->read_file_data(_subfile_info);
  }
}

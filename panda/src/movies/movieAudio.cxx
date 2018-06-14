/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieAudio.cxx
 * @author jyelon
 * @date 2007-07-02
 */

#include "movieAudio.h"
#include "movieAudioCursor.h"
#include "config_movies.h"
#include "movieTypeRegistry.h"

TypeHandle MovieAudio::_type_handle;

/**
 * This constructor returns a null audio stream --- a stream of total silence,
 * at 8000 samples per second.  To get more interesting audio, you need to
 * construct a subclass of this class.
 */
MovieAudio::
MovieAudio(const std::string &name) :
  Namable(name)
{
}

/**
 *
 */
MovieAudio::
~MovieAudio() {
}

/**
 * Open this audio, returning a MovieAudioCursor
 */
PT(MovieAudioCursor) MovieAudio::
open() {
  return new MovieAudioCursor(this);
}

/**
 * Obtains a MovieAudio that references a file.  Just calls
 * MovieTypeRegistry::make_audio().
 */
PT(MovieAudio) MovieAudio::
get(const Filename &name) {
  MovieTypeRegistry *reg = MovieTypeRegistry::get_global_ptr();
  return reg->make_audio(name);
}
